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

#include <QObject>
#include <QSet>
#include <QHash>

#ifdef ENABLE_CRYPT
#include <QtCrypto>
#endif

class QXmppMessage;
class QXmppPresence;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class GlooxMessage;

#ifdef ENABLE_CRYPT
	class PgpManager;
#endif

	class CryptHandler : public QObject
	{
		Q_OBJECT

		ClientConnection *Conn_;

#ifdef ENABLE_CRYPT
		PgpManager *PGPManager_ = nullptr;
#endif

		QSet<QString> SignedPresences_;
		QSet<QString> SignedMessages_;
		QHash<QString, QString> EncryptedMessages_;
		QSet<QString> Entries2Crypt_;
	public:
		CryptHandler (ClientConnection*);

		void Init ();

		void HandlePresence (const QXmppPresence&, const QString&, const QString&);
		void ProcessOutgoing (QXmppMessage&, GlooxMessage*);
		void ProcessIncoming (QXmppMessage&);

#ifdef ENABLE_CRYPT
		PgpManager* GetPGPManager () const;
		bool SetEncryptionEnabled (const QString&, bool);
#endif
	private slots:
		void handleEncryptedMessageReceived (const QString&, const QString&);
		void handleSignedMessageReceived (const QString&);
		void handleSignedPresenceReceived (const QString&);
		void handleInvalidSignatureReceived (const QString&);
	};
}
}
}
