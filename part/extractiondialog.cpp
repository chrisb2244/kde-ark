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
#include "extractiondialog.h"

#include <KLocale>
#include <KIconLoader>

ExtractionDialogUI::ExtractionDialogUI( QWidget *parent )
	: QFrame( parent )
{
	setupUi( this );
}

ExtractionDialog::ExtractionDialog( QWidget *parent )
	: KDialog( parent )
{
	m_ui = new ExtractionDialogUI( this );
	setMainWidget( m_ui );
	setButtons( Ok | Cancel );
	setCaption( i18n( "Extract" ) );
	m_ui->iconLabel->setPixmap( DesktopIcon( "ark-extract" ) );
	m_ui->iconLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );

	m_ui->filesToExtractGroupBox->hide();
	m_ui->extractAllLabel->show();
}

ExtractionDialog::~ExtractionDialog()
{
	delete m_ui;
	m_ui = 0;
}

void ExtractionDialog::showSelectedFilesOption()
{
	m_ui->filesToExtractGroupBox->show();
	m_ui->extractAllLabel->hide();
}

#include "extractiondialog.moc"