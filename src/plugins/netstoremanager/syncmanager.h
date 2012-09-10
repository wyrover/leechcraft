/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QPointer>
#include <boost/graph/graph_concepts.hpp>

class QTimer;
class QThread;
class QStandardItem;

namespace LeechCraft
{
namespace NetStoreManager
{
	class ISupportFileListings;
	class FilesWatcher;
	class IStorageAccount;
	class AccountsManager;

	class SyncManager : public QObject
	{
		Q_OBJECT

		AccountsManager *AM_;
		QMap<QString, IStorageAccount*> Path2Account_;
		QTimer *Timer_;

		QThread *Thread_;
		FilesWatcher *FilesWatcher_;

		QMap<ISupportFileListings*, QMap<QString, QStringList>> Isfl2PathId_;
	public:
		SyncManager (AccountsManager *am, QObject *parent = 0);

		void Release ();
	private:

	public slots:
		void handleDirectoryAdded (const QVariantMap& dirs);
	private slots:
		void handleTimeout ();
		void handleUpdateExceptionsList ();

		void handleDirWasCreated (const QString& path);
		void handleFileWasCreated (const QString& path);
		void handleDirWasRemoved (const QString& path);
		void handleFileWasRemoved (const QString& path);
		void handleEntryWasRenamed (const QString& oldPath, const QString&  newPath);
		void handleEntryWasMoved (const QString& oldPath, const QString& newPath);
		void handleFileWasUpdated (const QString& path);

		void handleGotListing (const QList<QList<QStandardItem*>>&);
	};
}
}
