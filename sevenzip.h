/*

  ark -- archiver for the KDE project

  Copyright (C)

  2004: Henrique Pinto <henrique.pinto@kdemail.net>

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
#ifndef SEVENZIP_H
#define SEVENZIP_H 

#include "arch.h"

class SevenZipArch : public Arch
{
  Q_OBJECT
  public:
    SevenZipArch( ArkWidget *, const QString & );
    virtual ~SevenZipArch();
  
    virtual void open();
    virtual void create();

    virtual void addFile( const QStringList & );
    virtual void addDir( const QString & );

    virtual void remove( QStringList * );
    virtual void unarchFile( QStringList *, const QString & destDir = "",
                             bool viewFriendly = false );

  private:
    void setHeaders();
};

#endif // SEVENZIP_H