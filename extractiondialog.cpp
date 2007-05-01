/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 
 *
 * Copyright (C) 2005 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (C) 2002 Helio Chissini de Castro <helio@conectiva.com.br>
 * Copyright (C) 2001 Roberto Selbach Teixeira <maragato@conectiva.com.br>
 * Copyright (C) 2001 Corel Corporation (author: Michael Jarrett, <michaelj@corel.com>)
 * Copyright (C) 1999-2000 Corel Corporation (author: Emily Ezust <emilye@corel.com>)
 * Copyright (C) 1999 Francois-Xavier Duranceau <duranceau@kde.org>
 * Copyright (C) 1997-1999 Rob Palmbos <palm9744@kettering.edu>
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

#include "extractiondialog.h"

#include <QGroupBox>
#include <QLabel>
#include <QRadioButton>
#include <QHBoxLayout>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <khistorycombobox.h>
#include <klineedit.h>
#include <kurlpixmapprovider.h>
#include <kdebug.h>
#include <kvbox.h>

#include "arkutils.h"
#include "settings.h"

ExtractionDialog::ExtractionDialog( QWidget *parent,
                                    bool enableSelected,
                                    const KUrl& defaultExtractionDir,
                                    const QString &prefix,
                                    const QString &archiveName )
	: KDialog( parent),
	  m_selectedButton( 0 ), m_allButton( 0 ),
	  m_selectedOnly( enableSelected ), m_extractionDirectory( defaultExtractionDir ),
	  m_defaultExtractionDir( defaultExtractionDir.prettyUrl() ), m_prefix( prefix )
{
    setCaption( i18n( "Extract" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
	if ( !archiveName.isNull() )
	{
		setCaption( i18n( "Extract Files From %1", archiveName ) );
	}

	KVBox *vbox = new KVBox( this );
        setMainWidget( vbox );

	KHBox *header = new KHBox( vbox );
	header->layout()->setSpacing( 10 );

	QLabel *icon = new QLabel( header );
	icon->setPixmap( DesktopIcon( "ark-extract" ) );
	icon->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );

	if ( enableSelected )
	{
		KVBox *whichFiles = new KVBox( header );
		whichFiles->layout()->setSpacing( 2 );
		new QLabel( QString( "<qt><b><font size=\"+1\">%1</font></b></qt>" )
		            .arg( i18n( "Extract:" ) ), whichFiles );
		QWidget *filesGroup = new QWidget( whichFiles );
		QHBoxLayout *filesGroupLayout = new QHBoxLayout( filesGroup );
		m_selectedButton = new QRadioButton( i18n( "Selected files only" ), filesGroup );
                filesGroupLayout->addWidget(m_selectedButton);
		m_allButton      = new QRadioButton( i18n( "All files" ), filesGroup );
                filesGroupLayout->addWidget(m_allButton);

		m_selectedButton->setChecked( true );
	}
	else
	{
		new QLabel( QString( "<qt><b><font size=\"+2\">%1</font></b></qt>" )
		            .arg( i18n( "Extract all files" ) ), header );
	}

	KHBox *destDirBox = new KHBox( vbox );

	QLabel *destFolderLabel = new QLabel( i18n( "Destination folder: " ), destDirBox );
	destFolderLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

	KHistoryComboBox *combobox = new KHistoryComboBox( true, destDirBox );
	combobox->setPixmapProvider( new KUrlPixmapProvider );
	combobox->setHistoryItems( ArkSettings::extractionHistory() );
	destFolderLabel->setBuddy( combobox );

	KUrlCompletion *comp = new KUrlCompletion();
	comp->setReplaceHome( true );
	comp->setCompletionMode( KGlobalSettings::CompletionAuto );
	combobox->setCompletionObject( comp );
	combobox->setMaxCount( 20 );
	combobox->setInsertPolicy( QComboBox::AtTop );

	m_urlRequester = new KUrlRequester( combobox, destDirBox );
	m_urlRequester->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	m_urlRequester->setMode( KFile::Directory );

	if (!defaultExtractionDir.prettyUrl().isEmpty() )
	{
		m_urlRequester->setUrl( defaultExtractionDir.prettyUrl() + prefix );
	}

	m_viewFolderAfterExtraction = new QCheckBox( i18n( "Open destination folder after extraction" ), vbox );
	m_viewFolderAfterExtraction->setChecked( ArkSettings::openDestinationFolder() );

	connect( combobox, SIGNAL( returnPressed( const QString& ) ), combobox, SLOT( addToHistory( const QString& ) ) );
	connect( combobox->lineEdit(), SIGNAL( textChanged( const QString& ) ),
	         this, SLOT( extractDirChanged( const QString & ) ) );
}

ExtractionDialog::~ExtractionDialog()
{
	ArkSettings::setExtractionHistory( ( static_cast<KHistoryComboBox*>( m_urlRequester->comboBox() ) )->historyItems() );
	ArkSettings::setOpenDestinationFolder( m_viewFolderAfterExtraction->isChecked() );
}

void ExtractionDialog::accept()
{

	KUrlCompletion uc;
	uc.setReplaceHome( true );
	KUrl p( uc.replacedPath( m_urlRequester->comboBox()->currentText() ) );

	//if p isn't local KIO and friends will complain later on
	if ( p.isLocalFile() )
	{
		QFileInfo fi( p.path() );
		if ( !fi.isDir() && !fi.exists() )
		{
			QString ltext = i18n( "Create folder %1?", p.path());
			int createDir =  KMessageBox::questionYesNo( this, ltext, i18n( "Missing Folder" ) , KGuiItem(i18n("Create Folder")), KGuiItem(i18n("Do Not Create")));
			if( createDir == 4 )
			{
				return;
			}
			// create directory using filename, make sure it has trailing slash
			p.adjustPath( KUrl::AddTrailingSlash );
			if( !KStandardDirs::makeDir( p.path() ) )
			{
				KMessageBox::error( this, i18n( "The folder could not be created. Please check permissions." ) );
				return;
			}
		}
		if ( !ArkUtils::haveDirPermissions( p.path() ) )
		{
			KMessageBox::error( this, i18n( "You do not have write permission to this folder. Please provide another folder." ) );
			return;
		}
	}

	m_extractionDirectory = p;
	m_selectedOnly = m_selectedButton == 0? false : m_selectedButton->isChecked();

	// Determine what exactly should be added to the extraction combo list
	QString historyURL = p.prettyUrl();
	if ( historyURL == KUrl( m_defaultExtractionDir + m_prefix ).prettyUrl() )
	{
		historyURL = m_defaultExtractionDir;
	}

	KHistoryComboBox *combo = static_cast<KHistoryComboBox*>( m_urlRequester->comboBox() );
	// If the item was already in the list, delete it from the list and readd it at the top
	combo->removeFromHistory( historyURL );
	combo->addToHistory( historyURL );

	KDialog::accept();
}

void ExtractionDialog::extractDirChanged(const QString &text )
{
	enableButtonOk(!text.isEmpty());
}

#include "extractiondialog.moc"
// kate: space-indent off; tab-width 4;
