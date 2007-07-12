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
#ifndef ARCHIVEMODEL_H
#define ARCHIVEMODEL_H

#include <QAbstractItemModel>
#include "kerfuffle/arch.h"

class Arch;
class ArchiveDirNode;

class ArchiveModel: public QAbstractItemModel
{
	Q_OBJECT
	public:
		ArchiveModel( Arch *archive, QObject *parent = 0 );
		~ArchiveModel();

		QVariant data( const QModelIndex &index, int role ) const;
		Qt::ItemFlags flags( const QModelIndex &index ) const;
		QVariant headerData( int section, Qt::Orientation orientation,
		                     int role = Qt::DisplayRole ) const;
		QModelIndex index( int row, int column,
		                   const QModelIndex &parent = QModelIndex() ) const;
		QModelIndex parent( const QModelIndex &index ) const;
		int rowCount( const QModelIndex &parent = QModelIndex() ) const;
		int columnCount( const QModelIndex &parent = QModelIndex() ) const;

	private slots:
		void slotNewEntry( const ArchiveEntry& entry );

	private:
		Arch *m_archive;
		ArchiveDirNode *m_rootNode;
};

#endif // ARCHIVEMODEL_H