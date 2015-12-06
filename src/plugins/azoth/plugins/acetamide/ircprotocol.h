/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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
#include <interfaces/azoth/iprotocol.h>
#include <interfaces/azoth/imucprotocol.h>
#include <interfaces/azoth/iurihandler.h>

namespace LeechCraft
{
struct Entity;
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;

	class IrcProtocol : public QObject
					  , public IProtocol
					  , public IMUCProtocol
					  , public IURIHandler
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IProtocol
				LeechCraft::Azoth::IMUCProtocol
				LeechCraft::Azoth::IURIHandler)

		QObject *ParentProtocolPlugin_;
		QList<IrcAccount*> IrcAccounts_;
		QObject *ProxyObject_ = nullptr;
	public:
		IrcProtocol (QObject* = nullptr);
		virtual ~IrcProtocol ();

		void Prepare ();
		QObject* GetProxyObject () const;
		void SetProxyObject (QObject*);

		QObject* GetQObject ();
		ProtocolFeatures GetFeatures () const;
		QList<QObject*> GetRegisteredAccounts ();

		QObject* GetParentProtocolPlugin () const;
		QString GetProtocolName () const;
		QIcon GetProtocolIcon () const;
		QByteArray GetProtocolID () const;
		QList<QWidget*> GetAccountRegistrationWidgets (AccountAddOptions);
		void RegisterAccount (const QString&, const QList<QWidget*>&);
		QWidget* GetMUCJoinWidget ();
		void RemoveAccount (QObject*);

		void HandleURI (const QUrl&, QObject*);
		bool SupportsURI (const QUrl&) const;
	private:
		void RestoreAccounts ();
	private slots:
		void saveAccounts () const;
	signals:
		void accountAdded (QObject*);
		void accountRemoved (QObject*);
	};
};
};
};
