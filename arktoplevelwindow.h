//  -*- mode: c++; c-basic-offset: 4; -*-
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
*/

#ifndef ARKTOPLEVELWINDOW_H
#define ARKTOPLEVELWINDOW_H

#define ARK_VERSION "2.1.9"

// QT includes
#include <qstring.h>
#include <qpopupmenu.h>
#include <qtimer.h>

// KDE includes
#include <kmainwindow.h>
#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kprogress.h>

// ark includes
#include "arkwidget.h"

class
ArkTopLevelWindow: public KParts::MainWindow
{
    Q_OBJECT
public:
    ArkTopLevelWindow( QWidget *parent=0, const char *name=0 );
    virtual ~ArkTopLevelWindow();

    void setExtractOnly ( bool b );
    void extractTo( const KURL & targetDirectory, const KURL & archive, bool guessName );
    void addToArchive( const KURL::List & filesToAdd, const QString & cwd = QString::null,
                       const KURL & archive = KURL(), bool askForName = false );

public slots:
    void file_newWindow();
    void file_new();
    void openURL( const KURL & url );
    void file_open();
    void file_reload();
    void editToolbars();
    void window_close();
    void file_quit();
    void file_close();
    void slotNewToolbarConfig();
    void slotConfigureKeyBindings();
    virtual void saveProperties( KConfig* config );
    virtual void readProperties( KConfig* config );
    void slotSaveProperties();
    void slotArchivePopup( const QPoint &pPoint);
    void slotRemoveRecentURL( const KURL &url );
    void slotAddRecentURL( const KURL &url );
    void slotFixActionState( const bool & bHaveFiles );
    void slotDisableActions();
    void slotAddOpenArk( const KURL & _arkname );
    void slotRemoveOpenArk( const KURL & _arkname );

protected:
    virtual bool queryClose(); // SM

private: // methods
    // disabling/enabling of buttons and menu items
    void setupActions();
    void setupMenuBar();

    void newCaption(const QString & filename);
    bool arkAlreadyOpen( const KURL & url );

    KURL getOpenURL( bool addOnly = false , const QString & caption = QString::null,
                     const QString & startDir = QString::null,
                     const QString & suggestedName = QString::null );

    void startProgressDialog( const QString & text );

private slots:
    void slotProgress();

private: // data
    KParts::ReadWritePart *m_part;
    ArkWidget *m_widget; //the parts widget

    KAction *newWindowAction;
    KAction *newArchAction;
    KAction *openAction;
    KAction *closeAction;
    KAction *reloadAction;
    KRecentFilesAction *recent;

    //progress dialog for konqs service menus / commmand line
    KProgressDialog *progressDialog;
    QTimer *timer;
};

#endif /* ARKTOPLEVELWINDOW_H*/
