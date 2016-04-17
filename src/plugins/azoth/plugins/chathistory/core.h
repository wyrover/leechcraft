/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QSet>
#include <QVariantMap>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/ihavetabs.h>
#include "storagestructures.h"

template<typename>
class QFuture;

namespace LeechCraft
{
namespace Azoth
{
class IMessage;
class IProxyObject;

namespace ChatHistory
{
	template<typename T>
	class STGuard
	{
		std::shared_ptr<T> C_;
	public:
		STGuard ()
		: C_ (T::Instance ())
		{}
	};

	class StorageThread;

	class Core : public QObject
	{
		Q_OBJECT
		static std::shared_ptr<Core> InstPtr_;

		StorageThread *StorageThread_;
		ICoreProxy_ptr CoreProxy_;
		IProxyObject *PluginProxy_;
		QSet<QString> DisabledIDs_;

		TabClassInfo TabClass_;

		Core ();
	public:
		static std::shared_ptr<Core> Instance ();

		~Core ();

		TabClassInfo GetTabClass () const;

		void SetCoreProxy (ICoreProxy_ptr);
		ICoreProxy_ptr GetCoreProxy () const;

		void SetPluginProxy (QObject*);
		IProxyObject* GetPluginProxy () const;

		bool IsLoggingEnabled (QObject*) const;
		void SetLoggingEnabled (QObject*, bool);

		void Process (QObject*);
		void Process (QVariantMap);

		QFuture<QStringList> GetOurAccounts ();

		QFuture<UsersForAccountResult_t> GetUsersForAccount (const QString&);

		QFuture<ChatLogsResult_t> GetChatLogs (const QString& accountId, const QString& entryId,
				int backpages, int amount);

		void Search (const QString& accountId, const QString& entryId,
				const QString& text, int shift, bool cs);
		void Search (const QString& accountId, const QString& entryId, const QDateTime& dt);
		void GetDaysForSheet (const QString& accountId, const QString& entryId, int year, int month);
		void ClearHistory (const QString& accountId, const QString& entryId);

		void RegenUsersCache ();
	private:
		void LoadDisabled ();
		void SaveDisabled ();
	signals:

		void gotSearchPosition (const QString&, const QString&, int);

		void gotDaysForSheet (const QString& accountId, const QString& entryId,
				int year, int month, const QList<int>& days);
	};
}
}
}
