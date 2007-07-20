/*
 ark -- archiver for the KDE project

 Copyright (C) 1997-1999 Rob Palmbos <palm9744@kettering.edu>
 Copyright (C) 1999 Francois-Xavier Duranceau <duranceau@kde.org>
 Copyright (C) 1999-2000 Corel Corporation (author: Emily Ezust <emilye@corel.com>)
 Copyright (C) 2001 Corel Corporation (author: Michael Jarrett <michaelj@corel.com>)
 Copyright (C) 2003 Hans Petter Bieker <bieker@kde.org>

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

#ifndef ARKUTILS_H
#define ARKUTILS_H

#include <kio/global.h>

class QStringList;

namespace ArkUtils
{
	// Check if the dir is writable
	bool haveDirPermissions( const QString &strFile );

	// check if disk has enough space to accommodate (a) new file(s) of
	// the given size in the partition containing the given directory
	bool diskHasSpace( const QString &dir, KIO::filesize_t size );

	KIO::filesize_t getSizes( const QStringList &list );
}

#endif