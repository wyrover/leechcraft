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

#include "customwebview.h"
#include <cmath>
#include <limits>
#include <qwebframe.h>
#include <qwebinspector.h>
#include <QMenu>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFile>
#include <QWebElement>
#include <QWebHistory>
#include <QTextCodec>
#include <QMouseEvent>

#if QT_VERSION < 0x050000
#include <QWindowsStyle>
#endif

#include <QFileDialog>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/poshuku/ibrowserwidget.h"
#include "interfaces/poshuku/poshukutypes.h"
#include "core.h"
#include "customwebpage.h"
#include "xmlsettingsmanager.h"
#include "webviewsmoothscroller.h"
#include "webviewrendersettingshandler.h"

namespace LeechCraft
{
namespace Poshuku
{
	CustomWebView::CustomWebView (QWidget *parent)
	: QWebView (parent)
	, WebInspector_
	{
		new QWebInspector,
		[] (QWebInspector *insp)
		{
			insp->hide ();
			insp->deleteLater ();
		}
	}
	{
		Core::Instance ().GetPluginManager ()->RegisterHookable (this);

#if QT_VERSION < 0x050000
		QPalette p;
		if (p.color (QPalette::Window) != Qt::white)
			setPalette (QWindowsStyle ().standardPalette ());
#endif

		new WebViewSmoothScroller { this };
		new WebViewRenderSettingsHandler { this };

		CustomWebPage *page = new CustomWebPage (this);
		setPage (page);

		WebInspector_->setPage (page);

		connect (this,
				SIGNAL (urlChanged (const QUrl&)),
				this,
				SLOT (remakeURL (const QUrl&)));
		connect (page,
				SIGNAL (loadingURL (const QUrl&)),
				this,
				SLOT (remakeURL (const QUrl&)));
		connect (page,
				SIGNAL (saveFrameStateRequested (QWebFrame*, QWebHistoryItem*)),
				this,
				SLOT (handleFrameState (QWebFrame*, QWebHistoryItem*)),
				Qt::QueuedConnection);

		connect (this,
				SIGNAL (loadFinished (bool)),
				this,
				SLOT (handleLoadFinished (bool)));

		connect (page,
				SIGNAL (couldHandle (LeechCraft::Entity, bool*)),
				this,
				SIGNAL (couldHandle (LeechCraft::Entity, bool*)));
		connect (page,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
		connect (page,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (LeechCraft::Entity, int*, QObject**)));
		connect (page,
				SIGNAL (printRequested (QWebFrame*)),
				this,
				SIGNAL (printRequested (QWebFrame*)));
		connect (page,
				SIGNAL (windowCloseRequested ()),
				this,
				SIGNAL (closeRequested ()));
		connect (page,
				SIGNAL (storeFormData (PageFormsData_t)),
				this,
				SIGNAL (storeFormData (PageFormsData_t)));
	}

	void CustomWebView::SetBrowserWidget (IBrowserWidget *widget)
	{
		Browser_ = widget;
	}

	void CustomWebView::Load (const QUrl& url, QString title)
	{
		if (url.isEmpty () || !url.isValid ())
			return;

		if (url.scheme () == "javascript")
		{
			QVariant result = page ()->mainFrame ()->
				evaluateJavaScript (url.toString ().mid (11));
			if (result.canConvert (QVariant::String))
				setHtml (result.toString ());
			return;
		}

		emit navigateRequested (url);

		if (url.scheme () == "about")
		{
			if (url.path () == "plugins")
				NavigatePlugins ();
			else if (url.path () == "home")
				NavigateHome ();
			return;
		}

		if (title.isEmpty ())
			title = tr ("Loading...");
		remakeURL (url);
		emit titleChanged (title);
		load (url);
	}

	void CustomWebView::Load (const QNetworkRequest& req,
			QNetworkAccessManager::Operation op, const QByteArray& ba)
	{
		emit titleChanged (tr ("Loading..."));
		QWebView::load (req, op, ba);
	}

	QString CustomWebView::URLToProperString (const QUrl& url)
	{
		QString string = url.toString ();
		QWebElement equivs = page ()->mainFrame ()->
				findFirstElement ("meta[http-equiv=\"Content-Type\"]");
		if (!equivs.isNull ())
		{
			QString content = equivs.attribute ("content", "text/html; charset=UTF-8");
			const QString charset = "charset=";
			int pos = content.indexOf (charset);
			if (pos >= 0)
				PreviousEncoding_ = content.mid (pos + charset.length ()).toLower ();
		}

		if (PreviousEncoding_ != "utf-8" &&
				PreviousEncoding_ != "utf8" &&
				!PreviousEncoding_.isEmpty ())
			string = url.toEncoded ();

		return string;
	}

	void CustomWebView::mousePressEvent (QMouseEvent *e)
	{
		qobject_cast<CustomWebPage*> (page ())->SetButtons (e->buttons ());
		qobject_cast<CustomWebPage*> (page ())->SetModifiers (e->modifiers ());

		const bool mBack = e->button () == Qt::XButton1;
		const bool mForward = e->button () == Qt::XButton2;
		if (mBack || mForward)
		{
			pageAction (mBack ? QWebPage::Back : QWebPage::Forward)->trigger ();
			e->accept ();
			return;
		}

		QWebView::mousePressEvent (e);
	}

	void CustomWebView::contextMenuEvent (QContextMenuEvent *e)
	{
		const auto& r = page ()->mainFrame ()->hitTestContent (e->pos ());
		emit contextMenuRequested (mapToGlobal (e->pos ()),
				{
					r.isContentEditable (),
					page ()->selectedText (),
					r.linkUrl (),
					r.linkText (),
					r.imageUrl (),
					r.pixmap ()
				});
	}

	void CustomWebView::keyReleaseEvent (QKeyEvent *event)
	{
		if (event->matches (QKeySequence::Copy))
			pageAction (QWebPage::Copy)->trigger ();
		else
			QWebView::keyReleaseEvent (event);
	}

	void CustomWebView::NavigatePlugins ()
	{
		QFile pef (":/resources/html/pluginsenum.html");
		pef.open (QIODevice::ReadOnly);
		QString contents = QString (pef.readAll ())
			.replace ("INSTALLEDPLUGINS", tr ("Installed plugins"))
			.replace ("NOPLUGINS", tr ("No plugins installed"))
			.replace ("FILENAME", tr ("File name"))
			.replace ("MIME", tr ("MIME type"))
			.replace ("DESCR", tr ("Description"))
			.replace ("SUFFIXES", tr ("Suffixes"))
			.replace ("ENABLED", tr ("Enabled"))
			.replace ("NO", tr ("No"))
			.replace ("YES", tr ("Yes"));
		setHtml (contents);
	}

	void CustomWebView::NavigateHome ()
	{
		QFile file (":/resources/html/home.html");
		file.open (QIODevice::ReadOnly);
		QString data = file.readAll ();
		data.replace ("{pagetitle}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{title}",
				tr ("Welcome to LeechCraft!"));
		data.replace ("{body}",
				tr ("Welcome to LeechCraft, the integrated internet-client.<br />"
					"More info is available on the <a href='http://leechcraft.org'>"
					"project's site</a>."));

		QBuffer iconBuffer;
		iconBuffer.open (QIODevice::ReadWrite);
		QPixmap pixmap ("lcicons:/resources/images/poshuku.svg");
		pixmap.save (&iconBuffer, "PNG");

		data.replace ("{img}",
				QByteArray ("data:image/png;base64,") + iconBuffer.buffer ().toBase64 ());

		setHtml (data);
	}

	void CustomWebView::remakeURL (const QUrl& url)
	{
		emit urlChanged (URLToProperString (url));
	}

	void CustomWebView::handleLoadFinished (bool ok)
	{
		if (ok)
			remakeURL (url ());
	}

	void CustomWebView::handleFrameState (QWebFrame*, QWebHistoryItem*)
	{
		const auto& histUrl = page ()->history ()->currentItem ().url ();
		if (histUrl != url ())
			remakeURL (histUrl);
	}

	void CustomWebView::openLinkHere ()
	{
		Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
	}

	void CustomWebView::openLinkInNewTab ()
	{
		CustomWebView *view = Core::Instance ().MakeWebView (false);
		view->Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
	}

	void CustomWebView::saveLink ()
	{
		pageAction (QWebPage::DownloadLinkToDisk)->trigger ();
	}

	void CustomWebView::subscribeToLink ()
	{
		const auto& list = qobject_cast<QAction*> (sender ())->data ().toList ();
		Entity e = Util::MakeEntity (list.at (0),
				QString (),
				FromUserInitiated | OnlyHandle,
				list.at (1).toString ());
		emit gotEntity (e);
	}

	void CustomWebView::bookmarkLink ()
	{
		const auto& list = qobject_cast<QAction*> (sender ())->data ().toList ();
		emit addToFavorites (list.at (1).toString (),
				list.at (0).toUrl ().toString ());
	}

	void CustomWebView::copyLink ()
	{
		pageAction (QWebPage::CopyLinkToClipboard)->trigger ();
	}

	void CustomWebView::openImageHere ()
	{
		Load (qobject_cast<QAction*> (sender ())->data ().toUrl ());
	}

	void CustomWebView::openImageInNewTab ()
	{
		pageAction (QWebPage::OpenImageInNewWindow)->trigger ();
	}

	void CustomWebView::saveImage ()
	{
		pageAction (QWebPage::DownloadImageToDisk)->trigger ();
	}

	void CustomWebView::savePixmap ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not an action"
					<< sender ();
			return;
		}

		const QPixmap& px = action->property ("Poshuku/OrigPX").value<QPixmap> ();
		if (px.isNull ())
			return;

		const QUrl& url = action->property ("Poshuku/OrigURL").value<QUrl> ();
		const QString origName = url.scheme () == "data" ?
				QString () :
				QFileInfo (url.path ()).fileName ();

		QString filter;
		QString fname = QFileDialog::getSaveFileName (0,
				tr ("Save pixmap"),
				QDir::homePath () + '/' + origName,
				tr ("PNG image (*.png);;JPG image (*.jpg);;All files (*.*)"),
				&filter);

		if (fname.isEmpty ())
			return;

		if (QFileInfo (fname).suffix ().isEmpty ())
		{
			if (filter.contains ("png"))
				fname += ".png";
			else if (filter.contains ("jpg"))
				fname += ".jpg";
		}

		QFile file (fname);
		if (!file.open (QIODevice::WriteOnly))
		{
			emit gotEntity (Util::MakeNotification ("Poshuku",
						tr ("Unable to save the image. Unable to open file for writing: %1.")
							.arg (file.errorString ()),
						PCritical_));
			return;
		}

		const QString& suf = QFileInfo (fname).suffix ();
		const bool isLossless = suf.toLower () == "png";
		px.save (&file,
				suf.toUtf8 ().constData (),
				isLossless ? 0 : 100);
	}

	void CustomWebView::copyImage ()
	{
		pageAction (QWebPage::CopyImageToClipboard)->trigger ();
	}

	void CustomWebView::copyImageLocation ()
	{
		QString url = qobject_cast<QAction*> (sender ())->data ().toUrl ().toString ();
		QClipboard *cb = QApplication::clipboard ();
		cb->setText (url, QClipboard::Clipboard);
		cb->setText (url, QClipboard::Selection);
	}
}
}
