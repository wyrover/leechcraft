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

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/lmp/ilmpplugin.h>
#include "ui_checktab.h"

class QSortFilterProxyModel;

#if QT_VERSION < 0x050000
class QDeclarativeView;
#else
class QQuickWidget;
#endif

namespace LeechCraft
{
namespace LMP
{
namespace BrainSlugz
{
	class CheckModel;
	class Checker;

	class CheckTab : public QWidget
				   , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::CheckTab Ui_;
#if QT_VERSION < 0x050000
		QDeclarativeView * const CheckView_;
#else
		QQuickWidget * const CheckView_;
#endif

		const ILMPProxy_ptr LmpProxy_;
		const ICoreProxy_ptr CoreProxy_;
		const TabClassInfo TC_;
		QObject * const Plugin_;

		QToolBar * const Toolbar_;

		CheckModel * const Model_;
		QAbstractItemModel * const CheckedModel_;

		bool IsRunning_;
	public:
		CheckTab (const ILMPProxy_ptr&, const ICoreProxy_ptr&,
				const TabClassInfo& tc, QObject *plugin);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;
	private:
		void SetupToolbar ();
	private slots:
		void on_SelectAll__released ();
		void on_SelectNone__released ();
		void handleStart ();
		void handleCheckFinished ();
	signals:
		void removeTab (QWidget*);

		void runningStateChanged (bool);
		void checkStarted (Checker*);
	};
}
}
}
