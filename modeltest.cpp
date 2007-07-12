/*
 * ark -- archiver for the KDE project
 *
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
#include "kerfuffle/arch.h"
#include "archivemodel.h"
#include <QtGui>
#include <KApplication>
#include <kcmdlineargs.h>
#include <klocale.h>

int main( int argc, char **argv )
{
	KCmdLineArgs::init( argc,argv, "archivemodeltest", 0, ki18n( "archivemodeltest" ), 0 );
	KApplication app;

	qRegisterMetaType<ArchiveEntry>( "ArchiveEntry" );

	ArchiveModel model( Arch::archFactory( UNKNOWN_FORMAT, "/home/kde-devel/ark_test.tar.gz", "wololo" ) );
	QTreeView view;
	view.setModel( &model );
	view.show();

	return app.exec();
}