/*

   ark -- archiver for the KDE project

   Copyright (C) 2002-2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
   Copyright (C) 2003: Helio Chissini de Castro <helio@conectiva.com>

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

#ifndef ARKMAINWINDOW_H
#define ARKMAINWINDOW_H

// QT includes

#include <QTimer>

// KDE includes
#include <kxmlguiwindow.h>
#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <KRecentFilesAction>

class ArkWidget;

class
MainWindow: public KParts::MainWindow
{
	Q_OBJECT
	public:
		MainWindow( QWidget *parent=0 );
		virtual ~MainWindow();

	public slots:
		void openURL( const KUrl & url, bool tempFile = false );
		void file_open();
		void editToolbars();
		void window_close();
		void slotNewToolbarConfig();
		void slotConfigureKeyBindings();
		virtual void saveProperties( KConfigGroup &config );
		virtual void readProperties( KConfigGroup &config );
		void slotSaveProperties();
		void slotArchivePopup( const QPoint &pPoint);
		void slotRemoveRecentURL( const KUrl &url );
		void slotAddRecentURL( const KUrl &url );
		void slotFixActionState( const bool & bHaveFiles );
		void slotDisableActions();
		void slotAddOpenArk( const KUrl & _arkname );
		void slotRemoveOpenArk( const KUrl & _arkname );

	protected:
		virtual bool queryClose(); // SM


	private slots:
		void slotNew();
	private:
		// disabling/enabling of buttons and menu items
		void setupActions();
		void setupMenuBar();

		void newCaption(const QString & filename);
		bool arkAlreadyOpen( const KUrl & url );

		KParts::ReadWritePart *m_part;
		ArkWidget *m_widget; //the parts widget

		QAction *newArchAction;
		QAction *openAction;
		KRecentFilesAction *recent;
};

#endif /* ARKMAINWINDOW_H*/