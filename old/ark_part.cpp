/*

   Copyright (C) 2001 Macadamian Technologies Inc (author: Jian Huang <jian@macadamian.com>)
   Copyright (C) 2003 Georg Robbers <Georg.Robbers@urz.uni-hd.de>
   Copyright (C) 2005 Henrique Pinto <henrique.pinto@kdemail.net>

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

#include "ark_part.h"
#include "arkwidget.h"
#include "settings.h"
#include "filelistview.h"

#include <KTreeWidgetSearchLine>
#include <KDebug>
#include <KMenu>
#include <KMessageBox>
#include <KAboutData>
#include <KXMLGUIFactory>
#include <KParts/Factory>
#include <KParts/GenericFactory>
#include <KStatusBar>
#include <KIconLoader>
#include <KIO/NetAccess>
#include <KLocale>
#include <KStandardAction>
#include <KIcon>
#include <KActionCollection>

#include <QFile>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

typedef KParts::GenericFactory<ArkPart> ArkFactory;
K_EXPORT_COMPONENT_FACTORY( liboldarkpart, ArkFactory );

KAboutData *ArkPart::createAboutData()
{
	KAboutData *about = new KAboutData( "ark", 0, ki18n( "ark" ),
			"3.0",
			ki18n( "Ark KParts Component" ),
			KAboutData::License_GPL,
			ki18n(  "(c) 1997-2007, The Various Ark Developers" ) );
	about->addAuthor( ki18n( "Robert Palmbos" ),KLocalizedString(), "palm9744@kettering.edu" );
	about->addAuthor( ki18n( "Francois-Xavier Duranceau" ),KLocalizedString(), "duranceau@kde.org" );
	about->addAuthor( ki18n( "Corel Corporation (author: Emily Ezust)" ),KLocalizedString(),
			"emilye@corel.com" );
	about->addAuthor( ki18n( "Corel Corporation (author: Michael Jarrett)" ), KLocalizedString(),
			"michaelj@corel.com" );
	about->addAuthor( ki18n( "Jian Huang" ) );
	about->addAuthor(  ki18n( "Roberto Teixeira" ), KLocalizedString(), "maragato@kde.org" );

	return about;
}

ArkPart::ArkPart( QWidget *parentWidget, QObject *parent, const QStringList & )
	: KParts::ReadWritePart( parent )
{
	setComponentData( ArkFactory::componentData() );
	m_widget = new  ArkWidget( parentWidget );

	setWidget( m_widget );
	connect( m_widget, SIGNAL( fixActions() ), this, SLOT( fixEnables() ) );
	connect( m_widget, SIGNAL( disableAllActions() ), this, SLOT( disableActions() ) );
	connect( m_widget, SIGNAL( signalFilePopup( const QPoint& ) ), this, SLOT( slotFilePopup( const QPoint& ) ) );
	connect( m_widget, SIGNAL( setWindowCaption( const QString & ) ), this, SIGNAL( setWindowCaption( const QString & ) ) );
	connect( m_widget, SIGNAL( removeRecentURL( const KUrl & ) ), this, SIGNAL( removeRecentURL( const KUrl & ) ) );
	connect( m_widget, SIGNAL( addRecentURL( const KUrl & ) ), this, SIGNAL( addRecentURL( const KUrl & ) ) );

	setXMLFile( "oldark_part.rc" );

	setupActions();

	m_ext = new ArkBrowserExtension( this, "ArkBrowserExtension" );
	connect( m_widget, SIGNAL( openUrlRequest( const KUrl & ) ),
			m_ext, SLOT( slotOpenUrlRequested( const KUrl & ) ) );

	m_bar = new ArkStatusBarExtension( this );
	connect( m_widget, SIGNAL( setStatusBarText( const QString & ) ), m_bar,
			SLOT( slotSetStatusBarText( const QString & ) ) );
	connect( m_widget, SIGNAL( setStatusBarSelectedFiles( const QString & ) ), m_bar,
			SLOT( slotSetStatusBarSelectedFiles( const QString & ) ) );
	connect( m_widget, SIGNAL( setBusy( const QString & ) ), m_bar,
			SLOT( slotSetBusy( const QString & ) ) );
	connect( m_widget, SIGNAL( setReady() ), m_bar,
			SLOT( slotSetReady() ) );
	connect( this, SIGNAL( started(KIO::Job*) ), SLOT( transferStarted(KIO::Job*) ) );
	connect( this, SIGNAL( completed() ), SLOT( transferCompleted() ) );
	connect( this, SIGNAL( canceled(const QString&) ),
			SLOT( transferCanceled(const QString&) ) );

	setProgressInfoEnabled( false );
}

ArkPart::~ArkPart()
{
}

void ArkPart::setupActions()
{
	addFileAction = actionCollection()->addAction("addfile");
	addFileAction->setIcon(KIcon("ark-addfile"));
	addFileAction->setText(i18n("Add &File..."));
	connect(addFileAction, SIGNAL(triggered(bool)), m_widget, SLOT(action_add()));

	addDirAction = actionCollection()->addAction("adddir");
	addDirAction->setText(i18n("Add Folde&r..."));
	addDirAction->setIcon(KIcon("ark-adddir"));
	connect(addDirAction, SIGNAL(triggered(bool)), m_widget, SLOT(action_add_dir()));

	extractAction =actionCollection()->addAction("extract");
	extractAction->setText(i18n("E&xtract..."));
	extractAction->setIcon(KIcon("ark-extract"));
	connect(extractAction, SIGNAL(triggered(bool)), m_widget, SLOT(action_extract()));

	deleteAction  = new KAction(KIcon("ark-delete"), i18n("De&lete"), this);
	actionCollection()->addAction("delete", deleteAction );
	connect(deleteAction, SIGNAL(triggered(bool)), m_widget, SLOT(action_delete()));

	viewAction = actionCollection()->addAction("view");
	viewAction->setText(i18nc("to view something","&View"));
	viewAction->setIcon(KIcon("ark-view"));
	connect(viewAction, SIGNAL(triggered(bool)), m_widget, SLOT(action_view()));

	openWithAction  = new KAction(i18n("&Open With..."), this);
	actionCollection()->addAction("open_with", openWithAction );
	connect(openWithAction, SIGNAL(triggered(bool) ), m_widget, SLOT(slotOpenWith()));

	selectAllAction = KStandardAction::selectAll(m_widget->fileList(), SLOT(selectAll()), actionCollection());
	actionCollection()->addAction("select_all", selectAllAction);

	deselectAllAction  = new KAction(i18n("&Clear Selection"), this);
	actionCollection()->addAction("deselect_all", deselectAllAction );
	connect(deselectAllAction, SIGNAL(triggered(bool) ), m_widget->fileList(), SLOT(clearSelection()));

	invertSelectionAction  = new KAction(i18n("&Invert Selection"), this);
	actionCollection()->addAction("invert_selection", invertSelectionAction );
	connect(invertSelectionAction, SIGNAL(triggered(bool) ), m_widget->fileList(), SLOT(invertSelection()));

	showSearchBar  = new KToggleAction(i18n("Show Search Bar"), this);
	actionCollection()->addAction("options_show_search_bar", showSearchBar );
	showSearchBar->setChecked( ArkSettings::showSearchBar() );
	connect( showSearchBar, SIGNAL( toggled( bool ) ), m_widget, SLOT( slotShowSearchBarToggled( bool ) ) );


	QAction *preferences = KStandardAction::preferences(m_widget, SLOT(showSettings()), actionCollection());
	preferences->setText(i18n( "Configure &Ark..." ));
	actionCollection()->addAction( "preferences", preferences );

	initialEnables();
}


void ArkPart::fixEnables()
{
	bool bHaveFiles = ( m_widget->getNumFilesInArchive() > 0 );
	bool bReadOnly = false;
	bool bAddDirSupported = true;
	if ( m_widget->archiveType() == ZOO_FORMAT || m_widget->archiveType() == AA_FORMAT
	     || m_widget->archiveType() == COMPRESSED_FORMAT )
	{
		bAddDirSupported = false;
	}

	if ( m_widget->archive() )
	{
		bReadOnly = m_widget->archive()->isReadOnly();
	}

	selectAllAction->setEnabled( bHaveFiles );
	deselectAllAction->setEnabled( bHaveFiles );
	invertSelectionAction->setEnabled( bHaveFiles );

	deleteAction->setEnabled( bHaveFiles && m_widget->numSelectedFiles() > 0
	                          && m_widget->archive() && !bReadOnly );
	addFileAction->setEnabled( m_widget->isArchiveOpen() && !bReadOnly );
	addDirAction->setEnabled( m_widget->isArchiveOpen() && !bReadOnly && bAddDirSupported );
	extractAction->setEnabled( bHaveFiles );
	m_widget->searchBar()->setEnabled( bHaveFiles );

	bool isAnEntrySelected = ( bHaveFiles && (m_widget->numSelectedFiles() == 1)
	                           && (m_widget->fileList()->currentItem()->childCount() == 0) );
	viewAction->setEnabled( isAnEntrySelected );
	openWithAction->setEnabled( isAnEntrySelected );
	emit fixActionState( bHaveFiles );
}

void ArkPart::initialEnables()
{
	selectAllAction->setEnabled( false );
	deselectAllAction->setEnabled( false );
	invertSelectionAction->setEnabled( false );

	viewAction->setEnabled( false );

	deleteAction->setEnabled( false );
	extractAction->setEnabled( false );
	addFileAction->setEnabled( false );
	addDirAction->setEnabled( false );
	openWithAction->setEnabled( false );

	m_widget->searchBar()->setEnabled( false );
}

void ArkPart::disableActions()
{
	selectAllAction->setEnabled( false );
	deselectAllAction->setEnabled( false );
	invertSelectionAction->setEnabled( false );

	viewAction->setEnabled( false );
	deleteAction->setEnabled( false );
	extractAction->setEnabled( false );
	addFileAction->setEnabled( false );
	addDirAction->setEnabled( false );
	openWithAction->setEnabled( false );
	m_widget->searchBar()->setEnabled( false );
}

bool ArkPart::openURL( const KUrl & url )
{
	m_widget->setRealURL( url );
	return KParts::ReadWritePart::openUrl( KIO::NetAccess::mostLocalUrl( url, m_widget ) );
}

bool ArkPart::openFile()
{
	KUrl url;
	url.setPath( localFilePath() );
	if( !QFile::exists( localFilePath() ) )
	{
		emit setWindowCaption( QString() );
		emit removeRecentURL( m_widget->realURL() );
		return false;
	}
	closeArchive();
	emit addRecentURL( m_widget->realURL() );
	m_widget->file_open( url );
	return true;
}

bool ArkPart::saveFile()
{
	return true;
}

bool ArkPart::closeArchive()
{
	m_widget->file_close();
	m_widget->setModified( false );
	return ReadWritePart::closeUrl();
}

bool ArkPart::closeUrl()
{
	return closeArchive();
}

void ArkPart::slotFilePopup( const QPoint &pPoint )
{
	if ( factory() )
	{
		static_cast<KMenu *>(factory()->container("file_popup", this))->popup(pPoint);
	}
}

void ArkPart::transferStarted( KIO::Job *job )
{
	m_job = job;

	m_bar->slotSetBusy( i18n( "Downloading %1...", url().prettyUrl() ), (job != 0), (job != 0) );

	if ( job )
	{
		disableActions();
		connect( job, SIGNAL( percent(KJob*, unsigned long) ), SLOT( progressInformation(KJob*, unsigned long) ) );
		connect( m_bar->cancelButton(), SIGNAL( clicked() ), SLOT( cancelTransfer() ) );
	}
}

void ArkPart::transferCompleted()
{
	if ( m_job )
	{
		disconnect( m_job, SIGNAL( percent(KJob*, unsigned long) ),
		            this, SLOT( progressInformation(KJob*, unsigned long) ) );
		m_job = 0;
	}

	m_bar->slotSetReady();
}

void ArkPart::transferCanceled( const QString& errMsg )
{
	m_job = 0;
	if ( !errMsg.isEmpty() )
	{
		KMessageBox::error( m_widget, errMsg );
	}
	initialEnables();
	m_bar->slotSetReady();
}

void ArkPart::progressInformation( KJob *, unsigned long progress )
{
	m_bar->setProgress( progress );
}

void ArkPart::cancelTransfer()
{
	disconnect( m_bar->cancelButton(), SIGNAL( clicked() ),
	            this, SLOT( cancelTransfer() ) );
	if ( m_job )
	{
		m_job->kill( KJob::EmitResult  );
		transferCanceled( QString() );
	}
}

ArkBrowserExtension::ArkBrowserExtension( KParts::ReadOnlyPart * parent, const char * /*name*/ )
	: KParts::BrowserExtension( parent )
{
}

void ArkBrowserExtension::slotOpenUrlRequested( const KUrl & url )
{
	emit openUrlRequest( url, KParts::URLArgs() );
}

ArkStatusBarExtension::ArkStatusBarExtension( KParts::ReadWritePart * parent )
	: KParts::StatusBarExtension( parent ),
	  m_bBusy( false ),
	  m_pStatusLabelSelect( 0 ),
	  m_pStatusLabelTotal( 0 ),
	  m_pBusyText( 0 ),
	  m_cancelButton( 0 ),
	  m_pProgressBar( 0 ),
	  m_pTimer( 0 )
{
}

ArkStatusBarExtension::~ArkStatusBarExtension()
{
}

void ArkStatusBarExtension::setupStatusBar()
{
	if ( m_pTimer                      // setup already done
	     || !statusBar() )
	{
		return;
	}

	m_pTimer = new QTimer( this );
	connect( m_pTimer, SIGNAL( timeout() ), this, SLOT( slotProgress() ) );

	m_pStatusLabelTotal = new QLabel( statusBar() );
	m_pStatusLabelTotal->setFrameStyle( QFrame::NoFrame );
	m_pStatusLabelTotal->setAlignment( Qt::AlignRight );
	m_pStatusLabelTotal->setText( i18n( "Total: 0 files" ) );

	m_pStatusLabelSelect = new QLabel( statusBar() );
	m_pStatusLabelSelect->setFrameStyle( QFrame::NoFrame );
	m_pStatusLabelSelect->setAlignment( Qt::AlignLeft );
	m_pStatusLabelSelect->setText(i18n( "0 files selected" ) );

	m_cancelButton = new QPushButton( SmallIcon( "cancel" ), QString(), statusBar() );
	m_cancelButton->setObjectName( "Status Bar cancel button" );

	addStatusBarItem( m_pStatusLabelSelect, 3000, false );
	addStatusBarItem( m_pStatusLabelTotal, 3000, false );
}

void ArkStatusBarExtension::slotSetStatusBarText( const QString & text )
{
	if ( statusBar() )
	{
		setupStatusBar();
		m_pStatusLabelTotal->setText( text );
	}
}

void ArkStatusBarExtension::slotSetStatusBarSelectedFiles( const QString & text )
{
	if ( statusBar() )
	{
		setupStatusBar();
		m_pStatusLabelSelect->setText( text );
	}
}

void ArkStatusBarExtension::slotSetBusy( const QString & text, bool showCancelButton, bool detailedProgress )
{
	if ( m_bBusy || !statusBar() )
		return;

	setupStatusBar();
	if ( !m_pBusyText )
	{
		m_pBusyText = new QLabel( statusBar() );

		m_pBusyText->setAlignment( Qt::AlignLeft );
		m_pBusyText->setFrameStyle( QFrame::Panel | QFrame::Raised );
	}

	if ( !m_pProgressBar )
	{
		m_pProgressBar = new QProgressBar( statusBar() );
		m_pProgressBar->setFixedHeight( m_pBusyText->fontMetrics().height() );
	}

	if ( !detailedProgress )
	{
		m_pProgressBar->setMaximum( 0 );
		m_pProgressBar->setTextVisible( false );
	}
	else
	{
		m_pProgressBar->setMaximum(100);
		m_pProgressBar->setTextVisible( true );
	}

	m_pBusyText->setText( text );

	removeStatusBarItem( m_pStatusLabelSelect );
	removeStatusBarItem( m_pStatusLabelTotal );

	addStatusBarItem( m_pBusyText, 5, true );
	addStatusBarItem( m_pProgressBar, 1, true );
	if ( showCancelButton )
	{
		addStatusBarItem( m_cancelButton, 0, true );
	}

	if ( !detailedProgress )
	{
		m_pTimer->start( 200);
		m_pTimer->setSingleShot(false);
	}
	m_bBusy = true;
}

void ArkStatusBarExtension::slotSetReady()
{
	if ( !m_bBusy || !statusBar() )
		return;

	setupStatusBar();
	m_pTimer->stop();
	m_pProgressBar->setValue( 0 );

	removeStatusBarItem( m_pBusyText );
	removeStatusBarItem( m_pProgressBar );
	removeStatusBarItem( m_cancelButton );

	addStatusBarItem( m_pStatusLabelSelect, 3000, false );
	addStatusBarItem( m_pStatusLabelTotal, 3000, false );

	m_bBusy = false;
}

void ArkStatusBarExtension::slotProgress()
{
	if ( !statusBar() )
		return;

	setupStatusBar();
	m_pProgressBar->setValue( m_pProgressBar->value() + 4 );
}

void ArkStatusBarExtension::setProgress( unsigned long progress )
{
	if ( m_pProgressBar && ( m_pProgressBar->maximum() != 0 ) )
	{
		m_pProgressBar->setValue( progress );
	}
}

#include "ark_part.moc"