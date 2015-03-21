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

#include "appinfomanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImage>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include "vkconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Murm
{
	AppInfoManager::AppInfoManager (QNetworkAccessManager *nam, VkConnection *conn, QObject *parent)
	: QObject { parent }
	, NAM_ { nam }
	, Conn_ { conn }
	{
	}

	bool AppInfoManager::HasAppInfo (qulonglong appId) const
	{
		return AppId2Info_.contains (appId);
	}

	AppInfo AppInfoManager::GetAppInfo (qulonglong appId) const
	{
		return AppId2Info_.value (appId);
	}

	void AppInfoManager::CacheAppInfo (const QList<AppInfo>& infos)
	{
		for (const auto& info : infos)
			CacheAppInfo (info.AppId_);
	}

	QImage AppInfoManager::GetAppImage (const AppInfo& info) const
	{
		return Url2Image_ [info.Icon25_];
	}

	void AppInfoManager::CacheAppInfo (qulonglong appId)
	{
		if (!appId ||
				PendingAppInfos_.contains (appId))
			return;

		if (AppId2Info_.contains (appId))
		{
			emit gotAppInfo (AppId2Info_.value (appId));
			return;
		}

		PendingAppInfos_ << appId;

		Conn_->GetAppInfo (appId,
				[this, appId] (const AppInfo& info)
				{
					AppId2Info_ [info.AppId_] = info;
					PendingAppInfos_.remove (appId);

					if (info.Icon25_.isValid ())
						CacheImage (info.Icon25_, info.AppId_);
					else
						emit gotAppInfo (info);
				});
	}

	void AppInfoManager::CacheImage (const QUrl& url, qulonglong id)
	{
		if (!url.isValid ())
			return;

		if (PendingUrls_.contains (url) ||
				Url2Image_.contains (url))
			return;

		PendingUrls_ << url;

		const auto reply = NAM_->get (QNetworkRequest { url });
		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, reply, url, id]
			{
				reply->deleteLater ();
				if (reply->error () != QNetworkReply::NoError)
				{
					qWarning () << Q_FUNC_INFO
							<< reply->errorString ();
					return;
				}

				const auto& img = QImage::fromData (reply->readAll ())
						.scaled (24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
				Url2Image_ [url] = img;

				emit gotAppInfo (AppId2Info_ [id]);
			},
			reply,
			SIGNAL (finished ()),
			this
		};
	}
}
}
}
