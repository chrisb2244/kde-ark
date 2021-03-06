/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv (at@at) stud.ntnu.no>
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

#include "extractHereDndPlugin.h"
#include "logging.h"
#include "batchextract.h"
#include "kerfuffle/archive_kerfuffle.h"

#include <QAction>
#include <QDebug>
#include <KPluginFactory>
#include <KPluginLoader>
#include <kfileitemlistproperties.h>
#include <KLocalizedString>

Q_LOGGING_CATEGORY(ARK, "ark.extracthere", QtWarningMsg)

K_PLUGIN_FACTORY_WITH_JSON(ExtractHereDndPluginFactory, "ark_dndextract.json",
                           registerPlugin<ExtractHereDndPlugin>();)

void ExtractHereDndPlugin::slotTriggered()
{
    qCDebug(ARK) << "Preparing job";
    BatchExtract *batchJob = new BatchExtract();

    batchJob->setAutoSubfolder(true);
    batchJob->setDestinationFolder(m_dest.toDisplayString(QUrl::PreferLocalFile));
    batchJob->setPreservePaths(true);
    foreach(const QUrl& url, m_urls) {
        batchJob->addInput(url);
    }

    qCDebug(ARK) << "Starting job";
    batchJob->start();
}

ExtractHereDndPlugin::ExtractHereDndPlugin(QObject* parent, const QVariantList&)
        : KIO::DndPopupMenuPlugin(parent)
{
}

QList<QAction *> ExtractHereDndPlugin::setup(const KFileItemListProperties& popupMenuInfo,
                                             const QUrl& destination)
{
    QList<QAction *> actionList;

    const QString extractHereMessage = i18nc("@action:inmenu Context menu shown when an archive is being drag'n'dropped", "Extract here");

    if (!Kerfuffle::supportedMimeTypes().contains(popupMenuInfo.mimeType())) {
        qCWarning(ARK) << popupMenuInfo.mimeType() << "is not a supported mimetype";
        return actionList;
    }

    qCDebug(ARK) << "Plugin executed";

    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("archive-extract")),
                                  extractHereMessage, NULL);
    connect(action, &QAction::triggered, this, &ExtractHereDndPlugin::slotTriggered);

    actionList.append(action);
    m_dest = destination;
    m_urls = popupMenuInfo.urlList();

    return actionList;
}

#include "extractHereDndPlugin.moc"
