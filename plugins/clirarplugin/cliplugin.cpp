/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2010-2011,2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "cliplugin.h"
#include "app/logging.h"
#include "kerfuffle/kerfuffle_export.h"

#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

#include <KPluginFactory>

Q_LOGGING_CATEGORY(KERFUFFLE_PLUGIN, "ark.kerfuffle.clirar", QtWarningMsg)

using namespace Kerfuffle;
K_PLUGIN_FACTORY( CliPluginFactory, registerPlugin< CliPlugin >(); )
CliPlugin::CliPlugin(QObject *parent, const QVariantList& args)
        : CliInterface(parent, args)
        , m_parseState(ParseStateTitle)
        , m_isUnrar5(false)
        , m_isPasswordProtected(false)
        , m_isMultiVolume(false)
        , m_isSolid(false)
        , m_remainingIgnoreLines(1) //The first line of UNRAR output is empty.
        , m_linesComment(0)
{
    qCDebug(KERFUFFLE_PLUGIN) << "Loaded cli_rar plugin";

    // Empty lines are needed for parsing output of unrar.
    setListEmptyLines(true);
}

CliPlugin::~CliPlugin()
{
}

void CliPlugin::resetParsing()
{
    m_parseState = ParseStateTitle;
    m_remainingIgnoreLines = 1;
    m_comment.clear();
}

// #272281: the proprietary unrar program does not like trailing '/'s
//          in directories passed to it when extracting only part of
//          the files in an archive.
QString CliPlugin::escapeFileName(const QString &fileName) const
{
    if (fileName.endsWith(QLatin1Char('/'))) {
        return fileName.left(fileName.length() - 1);
    }

    return fileName;
}

ParameterList CliPlugin::parameterList() const
{
    static ParameterList p;

    if (p.isEmpty()) {
        p[CaptureProgress] = true;
        p[ListProgram] = p[ExtractProgram] = QStringList() << QLatin1String( "unrar" );
        p[DeleteProgram] = p[AddProgram] = QStringList() << QLatin1String( "rar" );

        p[ListArgs] = QStringList() << QLatin1String("vt")
                                    << QLatin1String("-v")
                                    << QLatin1String("$PasswordSwitch")
                                    << QLatin1String("$Archive");
        p[ExtractArgs] = QStringList() << QLatin1String( "-kb" )
                                       << QLatin1String( "-p-" )
                                       << QLatin1String( "$PreservePathSwitch" )
                                       << QLatin1String( "$PasswordSwitch" )
                                       << QLatin1String( "$RootNodeSwitch" )
                                       << QLatin1String( "$Archive" )
                                       << QLatin1String( "$Files" );
        p[PreservePathSwitch] = QStringList() << QLatin1String( "x" )
                                              << QLatin1String( "e" );
        p[RootNodeSwitch] = QStringList() << QLatin1String( "-ap$Path" );
        p[PasswordSwitch] = QStringList() << QLatin1String( "-p$Password" );
        p[PasswordHeaderSwitch] = QStringList() << QLatin1String("-hp$Password");
        p[DeleteArgs] = QStringList() << QLatin1String( "d" )
                                      << QLatin1String( "$Archive" )
                                      << QLatin1String( "$Files" );
        p[FileExistsExpression] = QLatin1String("^\\[Y\\]es, \\[N\\]o, \\[A\\]ll, n\\[E\\]ver, \\[R\\]ename, \\[Q\\]uit $");
        p[FileExistsFileName] = QStringList() << QLatin1String("^(.+) already exists. Overwrite it")  // unrar 3 & 4
                                              << QLatin1String("^Would you like to replace the existing file (.+)$"); // unrar 5
        p[FileExistsInput] = QStringList() << QLatin1String( "Y" )  //overwrite
                                           << QLatin1String( "N" )  //skip
                                           << QLatin1String( "A" )  //overwrite all
                                           << QLatin1String( "E" )  //autoskip
                                           << QLatin1String( "Q" ); //cancel
        p[AddArgs] = QStringList() << QLatin1String( "a" )
                                   << QLatin1String( "$Archive" )
                                   << QLatin1String("$PasswordSwitch")
                                   << QLatin1String( "$Files" );
        p[PasswordPromptPattern] = QLatin1String("Enter password \\(will not be echoed\\) for");
        p[WrongPasswordPatterns] = QStringList() << QLatin1String("password incorrect") << QLatin1String("wrong password");
        p[ExtractionFailedPatterns] = QStringList() << QLatin1String( "CRC failed" )
                                                    << QLatin1String( "Cannot find volume" );
    }

    return p;
}

bool CliPlugin::readListLine(const QString &line)
{
    // Ignore number of lines corresponding to m_remainingIgnoreLines.
    if (m_remainingIgnoreLines > 0) {
        --m_remainingIgnoreLines;
        return true;
    }

    // Parse the title line, which contains the version of unrar.
    if (m_parseState == ParseStateTitle) {

        QRegularExpression rxVersionLine(QStringLiteral("^UNRAR (\\d+\\.\\d+)( beta \\d)? .*$"));
        QRegularExpressionMatch matchVersion = rxVersionLine.match(line);

        if (matchVersion.hasMatch()) {
            m_parseState = ParseStateComment;
            QString unrarVersion = matchVersion.captured(1);
            qCDebug(KERFUFFLE_PLUGIN) << "UNRAR version" << unrarVersion << "detected";
            if (unrarVersion.toFloat() >= 5) {
                m_isUnrar5 = true;
                qCDebug(KERFUFFLE_PLUGIN) << "Using UNRAR 5 parser";
            } else {
                qCDebug(KERFUFFLE_PLUGIN) << "Using UNRAR 4 parser";
            }
        }  else {
            // If the second line doesn't contain an UNRAR title, something
            // is wrong.
            qCCritical(KERFUFFLE_PLUGIN) << "Failed to detect UNRAR output.";
            return false;
        }

    // Or see what version of unrar we are dealing with and call specific
    // handler functions.
    } else if (m_isUnrar5) {
        handleUnrar5Line(line);
    } else {
        handleUnrar4Line(line);
    }

    return true;
}

void CliPlugin::handleUnrar5Line(const QString &line) {

    // Parses the comment field.
    if (m_parseState == ParseStateComment) {

        // RegExp matching end of comment field.
        QRegularExpression rxCommentEnd(QStringLiteral("^Archive: \\S+$"));

        if (rxCommentEnd.match(line).hasMatch()) {
            m_parseState = ParseStateHeader;
            m_comment = m_comment.trimmed();
            m_linesComment = m_comment.count(QLatin1Char('\n')) + 1;
            if (!m_comment.isEmpty()) {
                qCDebug(KERFUFFLE_PLUGIN) << "Found a comment with" << m_linesComment << "lines";
            }

        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }

        return;
    }

    // Parses the header, which is whatever is between the comment field
    // and the entries.
    else if (m_parseState == ParseStateHeader) {

        // "Details: " indicates end of header.
        if (line.startsWith(QStringLiteral("Details: "))) {
            ignoreLines(1, ParseStateEntryDetails);
            if (line.contains(QLatin1String("volume")) && !m_isMultiVolume) {
                m_isMultiVolume = true;
                qCDebug(KERFUFFLE_PLUGIN) << "Multi-volume archive detected";
            }
            if (line.contains(QLatin1String("solid")) && !m_isSolid) {
                m_isSolid = true;
                qCDebug(KERFUFFLE_PLUGIN) << "Solid archive detected";
            }
        }
        return;
    }

    // Parses the entry details for each entry.
    else if (m_parseState == ParseStateEntryDetails) {

        // For multi-volume archives there is a header between the entries in
        // each volume.
        if (line.startsWith(QLatin1String("Archive: "))) {
            m_parseState = ParseStateHeader;
            return;

        // Empty line indicates end of entry.
        } else if (line.trimmed().isEmpty() && !m_unrar5Details.isEmpty()) {
            handleUnrar5Entry();

        } else {

            // All detail lines should contain a colon.
            if (!line.contains(QLatin1Char(':'))) {
                qCWarning(KERFUFFLE_PLUGIN) << "Unrecognized line:" << line;
                return;
            }

            // The details are on separate lines, so we store them in the QHash
            // m_unrar5Details.
            m_unrar5Details.insert(line.section(QLatin1Char(':'), 0, 0).trimmed().toLower(),
                                   line.section(QLatin1Char(':'), 1).trimmed());
        }

        return;
    }
}

void CliPlugin::handleUnrar5Entry() {

    ArchiveEntry e;

    QString compressionRatio = m_unrar5Details.value(QStringLiteral("ratio"));
    compressionRatio.chop(1); // Remove the '%'
    e[Ratio] = compressionRatio;

    QString time = m_unrar5Details.value(QStringLiteral("mtime"));
    QDateTime ts = QDateTime::fromString(time, QStringLiteral("yyyy-MM-dd HH:mm:ss,zzz"));
    e[Timestamp] = ts;

    bool isDirectory = (m_unrar5Details.value(QStringLiteral("type")) == QLatin1String("Directory"));
    e[IsDirectory] = isDirectory;

    if (isDirectory && !m_unrar5Details.value(QStringLiteral("name")).endsWith(QLatin1Char('/'))) {
        m_unrar5Details[QStringLiteral("name")] += QLatin1Char('/');
    }

    QString compression = m_unrar5Details.value(QStringLiteral("compression"));
    int optionPos = compression.indexOf(QLatin1Char('-'));
    if (optionPos != -1) {
        e[Method] = compression.mid(optionPos);
        e[Version] = compression.left(optionPos).trimmed();
    } else {
        // No method specified.
        e[Method].clear();
        e[Version] = compression;
    }

    m_isPasswordProtected = m_unrar5Details.value(QStringLiteral("flags")).contains(QStringLiteral("encrypted"));
    e[IsPasswordProtected] = m_isPasswordProtected;

    e[FileName] = m_unrar5Details.value(QStringLiteral("name"));
    e[InternalID] = m_unrar5Details.value(QStringLiteral("name"));
    e[Size] = m_unrar5Details.value(QStringLiteral("size"));
    e[CompressedSize] = m_unrar5Details.value(QStringLiteral("packed size"));
    e[Permissions] = m_unrar5Details.value(QStringLiteral("attributes"));
    e[CRC] = m_unrar5Details.value(QStringLiteral("crc32"));

    if (e[Permissions].toString().startsWith(QLatin1Char('l'))) {
        e[Link] = m_unrar5Details.value(QStringLiteral("target"));
    }

    m_unrar5Details.clear();
    emit entry(e);
}

void CliPlugin::handleUnrar4Line(const QString &line) {

    // Parses the comment field.
    if (m_parseState == ParseStateComment) {

        // RegExp matching end of comment field.
        QRegularExpression rxCommentEnd(QStringLiteral("^(Solid archive|Archive|Volume) \\S+$"));

        if (rxCommentEnd.match(line).hasMatch()) {

            if (line.startsWith(QLatin1String("Volume")) && !m_isMultiVolume) {
                m_isMultiVolume = true;
                qCDebug(KERFUFFLE_PLUGIN) << "Multi-volume archive detected";
            }
            if (line.startsWith(QLatin1String("Solid archive")) && !m_isSolid) {
                m_isSolid = true;
                qCDebug(KERFUFFLE_PLUGIN) << "Solid archive detected";
            }

            m_parseState = ParseStateHeader;
            m_comment = m_comment.trimmed();
            m_linesComment = m_comment.count(QLatin1Char('\n')) + 1;
            if (!m_comment.isEmpty()) {
                qCDebug(KERFUFFLE_PLUGIN) << "Found a comment with" << m_linesComment << "lines";
            }

        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }

        return;
    }

    // Parses the header, which is whatever is between the comment field
    // and the entries.
    else if (m_parseState == ParseStateHeader) {

        // Horizontal line indicates end of header.
        if (line.startsWith(QStringLiteral("--------------------"))) {
            m_parseState = ParseStateEntryFileName;
        }
        return;
    }

    // Parses the entry name, which is on the first line of each entry.
    else if (m_parseState == ParseStateEntryFileName) {

        // Ignore empty lines.
        if (line.trimmed().isEmpty()) {
            return;
        }

        // Three types of subHeaders can be displayed for unrar 3 and 4.
        // STM has 4 lines, RR has 3, and CMT has lines corresponding to
        // length of comment field +3. We ignore the subheaders.
        QRegularExpression rxSubHeader(QStringLiteral("^Data header type: (CMT|STM|RR)$"));
        QRegularExpressionMatch matchSubHeader = rxSubHeader.match(line);
        if (matchSubHeader.hasMatch()) {
            qCDebug(KERFUFFLE_PLUGIN) << "SubHeader of type" << matchSubHeader.captured(1) << "found";
            if (matchSubHeader.captured(1) == QLatin1String("STM")) {
                ignoreLines(4, ParseStateEntryFileName);
            } else if (matchSubHeader.captured(1) == QLatin1String("CMT")) {
                ignoreLines(m_linesComment + 3, ParseStateEntryFileName);
            } else if (matchSubHeader.captured(1) == QLatin1String("RR")) {
                ignoreLines(3, ParseStateEntryFileName);
            }
            return;
        }

        // The entries list ends with a horizontal line, followed by a
        // single summary line or, for multi-volume archives, another header.
        if (line.startsWith(QStringLiteral("-----------------"))) {
            m_parseState = ParseStateHeader;
            return;

        // Encrypted files are marked with an asterisk.
        } else if (line.startsWith(QLatin1Char('*'))) {
            m_isPasswordProtected = true;
            m_unrar4Details.append(QString(line.trimmed()).remove(0, 1)); //Remove the asterisk

        // Entry names always start at the second position, so a line not
        // starting with a space is not an entry name.
        } else if (!line.startsWith(QLatin1Char(' '))) {
            qCWarning(KERFUFFLE_PLUGIN) << "Unrecognized line:" << line;
            return;

        // If we reach this, then we can assume the line is an entry name, so
        // save it, and move on to the rest of the entry details.
        } else {
            m_unrar4Details.append(line.trimmed());
        }

        m_parseState = ParseStateEntryDetails;

        return;
    }

    // Parses the remainder of the entry details for each entry.
    else if (m_parseState == ParseStateEntryDetails) {

        // If the line following an entry name is empty, we did something
        // wrong.
        Q_ASSERT(!line.trimmed().isEmpty());

        // If we reach a horizontal line, then the previous line was not an
        // entry name, so go back to header.
        if (line.startsWith(QStringLiteral("-----------------"))) {
            m_parseState = ParseStateHeader;
            return;
        }

        // In unrar 3 and 4 the details are on a single line, so we
        // pass a QStringList containing the details. We need to store
        // it due to symlinks (see below).
        m_unrar4Details.append(line.split(QLatin1Char(' '),
                                          QString::SkipEmptyParts));

        // The details line contains 9 fields, so m_unrar4Details
        // should now contain 9 + the filename = 10 strings. If not, this is
        // not an archive entry.
        if (m_unrar4Details.size() != 10) {
            m_parseState = ParseStateHeader;
            return;
        }

        // When unrar 3 and 4 list a symlink, they output an extra line
        // containing the link target. The extra line is output after
        // the line we ignore, so we first need to ignore one line.
        if (m_unrar4Details.at(6).startsWith(QLatin1Char('l'))) {
            ignoreLines(1, ParseStateLinkTarget);
            return;
        } else {
            handleUnrar4Entry();
        }

        // Unrar 3 & 4 show a third line for each entry, which contains
        // three details: Host OS, Solid, and Old. We can ignore this
        // line.
        ignoreLines(1, ParseStateEntryFileName);

        return;
    }

    // Parses a symlink target.
    else if (m_parseState == ParseStateLinkTarget) {

        m_unrar4Details.append(QString(line).remove(QStringLiteral("-->")).trimmed());
        handleUnrar4Entry();

        m_parseState = ParseStateEntryFileName;
        return;
    }
}

void CliPlugin::handleUnrar4Entry() {

    ArchiveEntry e;

    QDateTime ts = QDateTime::fromString(QString(m_unrar4Details.at(4) + QLatin1Char(' ') + m_unrar4Details.at(5)),
                                         QStringLiteral("dd-MM-yy hh:mm"));
    // Unrar 3 & 4 output dates with a 2-digit year but QDateTime takes it as
    // 19??. Let's take 1950 as cut-off; similar to KDateTime.
    if (ts.date().year() < 1950) {
        ts = ts.addYears(100);
    }
    e[Timestamp] = ts;

    bool isDirectory = ((m_unrar4Details.at(6).at(0) == QLatin1Char('d')) ||
                        (m_unrar4Details.at(6).at(1) == QLatin1Char('D')));
    e[IsDirectory] = isDirectory;

    if (isDirectory && !m_unrar4Details.at(0).endsWith(QLatin1Char('/'))) {
        m_unrar4Details[0] += QLatin1Char('/');
    }

    // Unrar reports the ratio as ((compressed size * 100) / size);
    // we consider ratio as (100 * ((size - compressed size) / size)).
    // If the archive is a multivolume archive, a string indicating
    // whether the archive's position in the volume is displayed
    // instead of the compression ratio.
    QString compressionRatio = m_unrar4Details.at(3);
    if ((compressionRatio == QStringLiteral("<--")) ||
        (compressionRatio == QStringLiteral("<->")) ||
        (compressionRatio == QStringLiteral("-->"))) {
        compressionRatio = QLatin1Char('0');
    } else {
        compressionRatio.chop(1); // Remove the '%'
    }
    e[Ratio] = compressionRatio;

    // TODO:
    // - Permissions differ depending on the system the entry was added
    //   to the archive.
    e[FileName] = m_unrar4Details.at(0);
    e[InternalID] = m_unrar4Details.at(0);
    e[Size] = m_unrar4Details.at(1);
    e[CompressedSize] = m_unrar4Details.at(2);
    e[Permissions] = m_unrar4Details.at(6);
    e[CRC] = m_unrar4Details.at(7);
    e[Method] = m_unrar4Details.at(8);
    e[Version] = m_unrar4Details.at(9);
    e[IsPasswordProtected] = m_isPasswordProtected;

    if (e[Permissions].toString().startsWith(QLatin1Char('l'))) {
        e[Link] = m_unrar4Details.at(10);
    }

    m_unrar4Details.clear();
    emit entry(e);
}

void CliPlugin::ignoreLines(int lines, ParseState nextState) {
    m_remainingIgnoreLines = lines;
    m_parseState = nextState;
}

#include "cliplugin.moc"
