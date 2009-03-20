/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2008 Claudio Bantaloukas <rockdreamer@gmail.com>
 * Copyright (C) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
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
#include "kerfuffle/cliinterface.h"
#include "kerfuffle/archivefactory.h"
#include <kdebug.h>

#include <QDir>

using namespace Kerfuffle;

class CliPlugin: public CliInterface
{
	public:
		explicit CliPlugin( const QString & filename, QObject *parent = 0 )
			: CliInterface( filename, parent ),
			m_state(0)
		{

		}

		virtual ~CliPlugin()
		{

		}

		virtual ParameterList parameterList() const
		{
			static ParameterList p;
			if (p.isEmpty()) {

				//p[CaptureProgress] = true;
				p[ListProgram] = p[ExtractProgram] = p[DeleteProgram] = p[AddProgram] = "7z";

				p[ListArgs] = QStringList() << "l" << "-slt" << "$Archive";
				p[ExtractArgs] = QStringList() << "$PreservePathSwitch" << "$RootNodeSwitch" << "$Archive" << "$Files";
				p[PreservePathSwitch] = QStringList() << "x" << "e";
				p[RootNodeSwitch] = QStringList() << "-w$Path";
				p[FileExistsExpression] = "^(.+) already exists. Overwrite it";

			}
			return p;
		}


		int m_state;
		ArchiveEntry m_currentArchiveEntry;

		bool readListLine(QString line)
		{
			switch (m_state)
			{
				case 0: // header
					if (line.startsWith("Listing archive:"))
					{
						kDebug( 1601 ) << "Archive name: " << line.right(line.size() - 16).trimmed() ;
					}
					else if (line.startsWith("----------"))
					{
						m_state = 1;
					}
					else if (line.contains("Error:"))
					{
						kDebug( 1601 ) << line.mid(6);
						//m_errorMessages << line.mid(6);
					}
					break;
				case 1: // beginning of a file detail
					if (line.startsWith("Path ="))
					{
						m_currentArchiveEntry.clear();
						QString entryFilename = QDir::fromNativeSeparators(line.mid( 6).trimmed());
						m_currentArchiveEntry[FileName] = entryFilename;
						m_currentArchiveEntry[InternalID] = entryFilename;
						m_state = 2;
					}
					break;

				case 2: // file details
					if (line.startsWith("Size = "))
					{
						m_currentArchiveEntry[ Size ] = line.mid( 7).trimmed();
					}
					else if (line.startsWith("Packed Size = "))
					{
						m_currentArchiveEntry[ CompressedSize ] = line.mid( 14).trimmed();
					}
					else if (line.startsWith("Modified = "))
					{
						QDateTime ts = QDateTime::fromString(line.mid(11).trimmed(), "yyyy-MM-dd hh:mm:ss");
						m_currentArchiveEntry[ Timestamp ] = ts;
					}
					else if (line.startsWith("Attributes = "))
					{
						QString attributes = line.mid(13).trimmed();

						bool isDirectory = attributes.startsWith('D');
						m_currentArchiveEntry[ IsDirectory ] = isDirectory;
						if (isDirectory)
						{
							QString directoryName = m_currentArchiveEntry[FileName].toString();
							if (!directoryName.endsWith('/'))
							{
								m_currentArchiveEntry[FileName] = m_currentArchiveEntry[InternalID] = directoryName + '/';
							}
						}

						m_currentArchiveEntry[ Permissions ] = attributes.mid(1);
					}
					else if (line.startsWith("CRC = "))
					{
						m_currentArchiveEntry[ CRC ] = line.mid(6).trimmed();
					}
					else if (line.startsWith("Method = "))
					{
						QString method = line.mid(9).trimmed();
						m_currentArchiveEntry[ Method ] = method;
					}
					else if (line.startsWith("Encrypted = ") && line.size() >= 13)
					{
						bool isPasswordProtected = (line.at(12) == '+');
						m_currentArchiveEntry[ IsPasswordProtected ] = isPasswordProtected;
					}
					else if (line.startsWith("Block = "))
					{
						if (m_currentArchiveEntry.contains(FileName))
						{
							entry(m_currentArchiveEntry);
						}

						m_state = 1;
					}
					break;

				default:
					break;
			}
			return true;
		}
};

KERFUFFLE_PLUGIN_FACTORY(CliPlugin)
