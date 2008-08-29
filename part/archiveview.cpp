/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2008 Harald Hvaal <haraldhv (at@at) stud.ntnu.no>
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

#include "archiveview.h"
#include <QHeaderView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <kdebug.h>


ArchiveView::ArchiveView(QWidget *parent)
	: QTreeView(parent)
{

}

void ArchiveView::setModel(QAbstractItemModel *model)
{
	kDebug (1601) ;
	QTreeView::setModel(model);
	setSelectionMode( QAbstractItemView::ExtendedSelection );
	setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	setAlternatingRowColors( true );
	setAnimated( true );
	setAllColumnsShowFocus( true );
	header()->setResizeMode(0, QHeaderView::ResizeToContents);

	//drag and drop
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::DragDrop);
}


void ArchiveView::dragEnterEvent ( QDragEnterEvent * event )
{
	//TODO: if no model, trigger some mechanism to create one automatically!
	kDebug(1601) << event;

	QTreeView::dragEnterEvent(event);
	return;
}

void ArchiveView::dropEvent ( QDropEvent * event )
{
	kDebug(1601) << event;
	QTreeView::dropEvent(event);
	return;
}

void ArchiveView::dragMoveEvent ( QDragMoveEvent * event )
{
	QTreeView::dragMoveEvent(event);
	if (event->mimeData()->hasFormat("text/uri-list"))
         event->acceptProposedAction();
	return;
	kDebug() << indexAt(event->pos());
}