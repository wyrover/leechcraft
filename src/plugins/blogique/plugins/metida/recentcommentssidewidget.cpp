/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "recentcommentssidewidget.h"
#include <QtDebug>
#include <QUrl>
#ifdef USE_QT5
	#include <QQmlContext>
	#include <QQuickItem>
#else
	#include <QDeclarativeContext>
#endif
#include <QGraphicsObject>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/util.h>
#include "core.h"
#include "ljaccount.h"
#include "recentcommentsmodel.h"
#include "recentcommentsview.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	RecentCommentsSideWidget::RecentCommentsSideWidget (QWidget *parent)
	: QWidget (parent)
	, LJAccount_ (0)
	, RecentCommentsModel_ (new RecentCommentsModel (this))
	{
		Ui_.setupUi (this);

		auto view = new RecentCommentsView (this);
#ifdef USE_QT5
		QWidget *container = QWidget::createWindowContainer (view, this);
		container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		container->setFocusPolicy(Qt::StrongFocus);
		view->setResizeMode (QQuickView::SizeRootObjectToView);
		Ui_.GridLayout_->addWidget (container);
#else
		view->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		Ui_.GridLayout_->addWidget (view);
#endif
		view->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ()
						.GetCoreProxy ()->GetColorThemeManager ()));
		view->rootContext ()->setContextProperty ("recentCommentsModel",
				RecentCommentsModel_);
		view->rootContext ()->setContextProperty ("recentCommentsView",
				view);
		view->setSource (QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML,
				"blogique/metida", "recentcomments.qml")));

		connect (view->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLinkActivated (QString)));
	}

	QString RecentCommentsSideWidget::GetName () const
	{
		return tr ("Recent comments");
	}

	SideWidgetType RecentCommentsSideWidget::GetWidgetType () const
	{
		return SideWidgetType::CustomSideWidget;
	}

	QVariantMap RecentCommentsSideWidget::GetPostOptions () const
	{
		return QVariantMap ();
	}

	void RecentCommentsSideWidget::SetPostOptions (const QVariantMap&)
	{
	}

	QVariantMap RecentCommentsSideWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void RecentCommentsSideWidget::SetCustomData (const QVariantMap&)
	{
	}

	void RecentCommentsSideWidget::SetAccount (QObject* accountObj)
	{
		LJAccount_ = qobject_cast<LJAccount*> (accountObj);
		if (LJAccount_)
			connect (accountObj,
					SIGNAL (gotRecentComments (QList<LJCommentEntry>)),
					this,
					SLOT (handleGotRecentComents (QList<LJCommentEntry>)),
					Qt::UniqueConnection);
	}

	void RecentCommentsSideWidget::handleGotRecentComents (const QList<LJCommentEntry>& comments)
	{
		for (auto comment : comments)
		{
			QStandardItem *item = new QStandardItem;
			item->setData (comment.NodeSubject_, RecentCommentsModel::NodeSubject);
			item->setData (comment.NodeUrl_, RecentCommentsModel::NodeUrl);
			item->setData (comment.Text_, RecentCommentsModel::CommentBody);
			item->setData (tr ("by %1 on %2")
						.arg (comment.PosterName_.isEmpty () ?
							tr ("Anonymous") :
							comment.PosterName_)
						.arg (comment.PostingDate_.toString (Qt::DefaultLocaleShortDate)),
					RecentCommentsModel::CommentInfo);

			RecentCommentsModel_->appendRow (item);
		}
	}

	void RecentCommentsSideWidget::handleLinkActivated (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (link,
				QString (),
				OnlyHandle | FromUserInitiated));
	}

}
}
}
