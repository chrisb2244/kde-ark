/*

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

// ark includes
#include "zip.h"
#include "arkwidget.h"
#include "settings.h"

// Qt includes
#include <QDir>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <k3process.h>

ZipArch::ZipArch( ArkWidget *_gui, const QString & _fileName )
  : Arch(  _gui, _fileName )
{
  m_archiver_program = "zip";
  m_unarchiver_program = "unzip";
  verifyUtilityIsAvailable( m_archiver_program, m_unarchiver_program );

  m_headerString = "----";
  m_repairYear = 9; m_fixMonth = 7; m_fixDay = 8; m_fixTime = 10;
  m_dateCol = 5;
  m_numCols = 7;

  m_archCols.append( ArchColumns( 1, QRegExp( "[0-9]+" ) ) );
  m_archCols.append( ArchColumns( 2, QRegExp( "[^\\s]+" ) ) );
  m_archCols.append( ArchColumns( 3, QRegExp( "[0-9]+" ) ) );
  m_archCols.append( ArchColumns( 4, QRegExp( "[0-9.]+%" ) ) );
  m_archCols.append( ArchColumns( 7, QRegExp( "[01][0-9]" ), 2 ) );
  m_archCols.append( ArchColumns( 8, QRegExp( "[0-3][0-9]" ), 2 ) );
  m_archCols.append( ArchColumns( 9, QRegExp( "[0-9][0-9]" ), 2 ) );
  m_archCols.append( ArchColumns( 10, QRegExp( "[0-9:]+" ), 6 ) );
  m_archCols.append( ArchColumns( 6, QRegExp( "[a-fA-F0-9]+ {2}" ) ) );
  m_archCols.append( ArchColumns( 0, QRegExp( "[^\\n]+" ), 4096 ) );

}

void ZipArch::setHeaders()
{
  ColumnList list;
  list.append( FILENAME_COLUMN );
  list.append( SIZE_COLUMN );
  list.append( METHOD_COLUMN );
  list.append( PACKED_COLUMN );
  list.append( RATIO_COLUMN );
  list.append( TIMESTAMP_COLUMN );
  list.append( CRC_COLUMN );

  emit headers( list );
}

void ZipArch::open()
{
  setHeaders();

  m_buffer = "";
  m_header_removed = false;
  m_finished = false;

  K3Process *kp = m_currentProcess = new K3Process;

  *kp << m_unarchiver_program << "-v" << m_filename;

  connect( kp, SIGNAL( receivedStdout(K3Process*, char*, int) ),
           SLOT( slotReceivedTOC(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( processExited(K3Process*) ),
           SLOT( slotOpenExited(K3Process*) ) );

  if ( !kp->start( K3Process::NotifyOnExit, K3Process::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigOpen( this, false, QString(), 0 );
  }
}


void ZipArch::create()
{
  emit sigCreate( this, true, m_filename,
                 Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void ZipArch::addDir( const QString & _dirName )
{
  if ( !_dirName.isEmpty() )
  {
    bool bOldRecVal = ArkSettings::rarRecurseSubdirs();
    // must be true for add directory - otherwise why would user try?
    ArkSettings::setRarRecurseSubdirs( true );

    QStringList list;
    list.append( _dirName );
    addFile( list );
    ArkSettings::setRarRecurseSubdirs( bOldRecVal ); // reset to old val
  }
}

void ZipArch::addFile( const QStringList &urls )
{
  K3Process *kp = m_currentProcess = new K3Process;
  kp->clearArguments();

  *kp << m_archiver_program;

  if ( !m_password.isEmpty() )
    *kp << "-P" << m_password;

  if ( ArkSettings::rarRecurseSubdirs() )
    *kp << "-r";

  if ( ArkSettings::rarStoreSymlinks() )
    *kp << "-y";

  if ( ArkSettings::forceMSDOS() )
    *kp << "-k";
  if ( ArkSettings::convertLF2CRLF() )
    *kp << "-l";

  if ( ArkSettings::replaceOnlyWithNewer() )
    *kp << "-u";

  *kp << m_filename;

  QStringList::ConstIterator iter;
  KUrl url( urls.first() );
  QDir::setCurrent( url.directory() );
  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KUrl fileURL( *iter );
    *kp << fileURL.fileName();
  }

  connect( kp, SIGNAL( receivedStdout(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( processExited(K3Process*) ),
           SLOT( slotAddExited(K3Process*) ) );

  if ( !kp->start( K3Process::NotifyOnExit, K3Process::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigAdd( false );
  }
}

void ZipArch::unarchFileInternal()
{
  // if fileList is empty, all files are extracted.
  // if destDir is empty, abort with error.
  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  K3Process *kp = m_currentProcess = new K3Process;
  kp->clearArguments();

  *kp << m_unarchiver_program;

  if ( !m_password.isEmpty() )
    *kp << "-P" << m_password;

  if ( ArkSettings::extractJunkPaths() && !m_viewFriendly )
    *kp << "-j" ;

  if ( ArkSettings::rarToLower() )
    *kp << "-L";

  if ( ArkSettings::extractOverwrite() )
    *kp << "-o";
  else
    *kp << "-n";

  *kp << m_filename;

  // if the list is empty, no filenames go on the command line,
  // and we then extract everything in the archive.
  if ( m_fileList )
  {
    QStringList::Iterator it;

    for ( it = m_fileList->begin(); it != m_fileList->end(); ++it )
    {
      *kp << (*it);
    }
  }

  *kp << "-d" << m_destDir;

  connect( kp, SIGNAL( receivedStdout(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( processExited(K3Process*) ),
           SLOT( slotExtractExited(K3Process*) ) );

  if ( !kp->start( K3Process::NotifyOnExit, K3Process::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigExtract( false );
  }
}

bool ZipArch::passwordRequired()
{
    return m_lastShellOutput.contains("unable to get password\n") || m_lastShellOutput.endsWith("password inflating\n") || m_lastShellOutput.contains("password incorrect--reenter:") || m_lastShellOutput.endsWith("incorrect password\n");
}

void ZipArch::remove( QStringList *list )
{
  if ( !list )
    return;

  K3Process *kp = m_currentProcess = new K3Process;
  kp->clearArguments();

  *kp << m_archiver_program << "-d" << m_filename;

  QStringList::Iterator it;
  for ( it = list->begin(); it != list->end(); ++it )
  {
    QString str = *it;
    *kp << str;
  }

  connect( kp, SIGNAL( receivedStdout(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(K3Process*, char*, int) ),
           SLOT( slotReceivedOutput(K3Process*, char*, int) ) );
  connect( kp, SIGNAL( processExited(K3Process*) ),
           SLOT( slotDeleteExited(K3Process*) ) );

  if ( !kp->start( K3Process::NotifyOnExit, K3Process::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigDelete( false );
  }
}

#include "zip.moc"
