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

#include <atomic>
#include <memory>
#include <functional>
#include <type_traits>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <interfaces/azoth/iclentry.h>
#include <util/sll/futures.h>
#include "threadexceptions.h"
#include "toxaccountconfiguration.h"

typedef struct Tox Tox;

namespace LeechCraft
{
namespace Azoth
{
namespace Sarin
{
	class CallManager;

	class ToxThread : public QThread
	{
		Q_OBJECT

		std::atomic_bool ShouldStop_ { false };

		const QString Name_;

		QByteArray ToxState_;

		const ToxAccountConfiguration Config_;

		EntryStatus Status_;

		QList<std::function<void (Tox*)>> FQueue_;
		QMutex FQueueMutex_;

		std::shared_ptr<Tox> Tox_;
		std::shared_ptr<CallManager> CallManager_;
	public:
		ToxThread (const QString& name, const QByteArray& toxState, const ToxAccountConfiguration&);
		~ToxThread ();

		CallManager* GetCallManager () const;

		EntryStatus GetStatus () const;
		void SetStatus (const EntryStatus&);

		void Stop ();
		bool IsStoppable () const;

		QFuture<QByteArray> GetToxId ();

		enum class AddFriendResult
		{
			Added,
			InvalidId,
			TooLong,
			NoMessage,
			OwnKey,
			AlreadySent,
			BadChecksum,
			NoSpam,
			NoMem,
			Unknown
		};

		struct FriendInfo
		{
			QByteArray Pubkey_;
			QString Name_;
			EntryStatus Status_;
		};

		QFuture<AddFriendResult> AddFriend (QByteArray, QString);
		void AddFriend (QByteArray);
		void RemoveFriend (const QByteArray&);

		QFuture<QByteArray> GetFriendPubkey (qint32);
		QFuture<FriendInfo> ResolveFriend (qint32);

		template<typename F>
		auto ScheduleFunction (const F& func)
		{
			QFutureInterface<decltype (func ({}))> iface;
			iface.reportStarted ();
			ScheduleFunctionImpl ([iface, func] (Tox *tox) mutable
					{
						try
						{
							Util::ReportFutureResult (iface, func, tox);
						}
#if QT_VERSION < 0x050000
						catch (const QtConcurrent::Exception& e)
#else
						catch (const QException& e)
#endif
						{
							iface.reportException (e);
							iface.reportFinished ();
						}
						catch (const std::exception& e)
						{
							iface.reportException (ToxException { e });
							iface.reportFinished ();
						}
					});
			return iface.future ();
		}
	private:
		void ScheduleFunctionImpl (const std::function<void (Tox*)>&);

		void SaveState ();

		void LoadFriends ();

		void HandleFriendRequest (const uint8_t*, const uint8_t*, uint16_t);
		void HandleNameChange (int32_t, const uint8_t*, uint16_t);
		void UpdateFriendStatus (int32_t);
		void HandleTypingChange (int32_t, bool);
	protected:
		virtual void run ();
	signals:
		void statusChanged (const EntryStatus&);

		void toxCreated (Tox*);

		void toxStateChanged (const QByteArray&);

		void gotFriend (qint32);
		void gotFriendRequest (const QByteArray& pubkey, const QString& msg);
		void removedFriend (const QByteArray& pubkey);

		void friendNameChanged (const QByteArray& pubkey, const QString&);

		void friendStatusChanged (const QByteArray& pubkey, const EntryStatus& status);

		void friendTypingChanged (const QByteArray& pubkey, bool isTyping);
	};
}
}
}
