//  -*-C++-*-           emacs magic for .h files
/*

 $Id $

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org

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
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


#ifndef ARCH_H
#define ARCH_H

// KDE includes
#include <kprocess.h>

// Qt includes
#include <qstring.h>

// ark includes
#include "arksettings.h"
#include "filelistview.h"

class Viewer;
class KProcess;

// The following class is the base class for all of the archive types.
// In order for it to work properly with the KProcess, you have to
// connect the ProcessExited signal appropriately before spawning
// the core operations. Then the signal that the process exited can
// be intercepted by the viewer (in ark, ArkWidget) and dealt with
// appropriately. See ZipArch for a good model. Don't use TarArch
// as a model - it's too complicated!

class Arch : public QObject
{
  Q_OBJECT
public:
  Arch( ArkSettings *_settings, Viewer *_viewer, const QString & _fileName );
  virtual ~Arch();
	
  virtual void open() = 0;
  virtual void create() = 0;
  virtual void remove(QStringList *) = 0;

  virtual int addFile(QStringList *) = 0;
  virtual int addDir(const QString &) = 0;

  // unarch the files in the list or all files if the list is empty
  virtual QString unarchFile(QStringList *, const QString & _destDir="") = 0;

  virtual int actionFlag() = 0;
	
  QString fileName() const { return m_filename; };
	
  enum EditProperties{ Add = 1, Delete = 2, Extract = 4,
    View = 8, Integrity = 16 };

  bool stderrIsError();

protected slots:
  void slotCancel();
  void slotStoreDataStdout(KProcess*, char*, int);
  void slotStoreDataStderr(KProcess*, char*, int);
  void slotOpenExited(KProcess*);
	
  void slotExtractExited(KProcess*);
  void slotDeleteExited(KProcess*);
  void slotAddExited(KProcess*);

signals:
  void sigOpen( bool, const QString &, int );
  void sigCreate( bool, const QString &, int );
  void sigDelete(bool);
  void sigExtract(bool);
  void sigAdd(bool);
	
protected:
  QString m_filename;
  QString m_shellErrorData;
  char m_buffer[1024];
  ArkSettings *m_settings;
  Viewer *m_gui;
  KProcess *m_kp;
};


enum AddRetCodes { SUCCESS, FAILURE, UNSUPDIR } ;

#endif /* ARCH_H */
