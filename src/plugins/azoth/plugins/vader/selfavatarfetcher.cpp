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

#include "selfavatarfetcher.h"
#include <QTimer>
#include <QStringList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include "avatarstimestampstorage.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	SelfAvatarFetcher::Urls::Urls (const QString& full)
	{
		auto split = full.split ('@', QString::SkipEmptyParts);
		if (split.size () != 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid full address"
					<< full;
			return;
		}

		auto& name = split [0];
		auto& domain = split [1];
		if (domain.endsWith (".ru"))
			domain.chop (3);

		const auto& base = "http://obraz.foto.mail.ru/" + domain + "/" + name + "/_mrimavatar";

		SmallUrl_ = base + "small";
		BigUrl_ = base + "big";
	}

	SelfAvatarFetcher::SelfAvatarFetcher (QNetworkAccessManager *nam,
			const QString& full, QObject *parent)
	: QObject { parent }
	, NAM_ { nam }
	, Timer_ { new QTimer { this } }
	, FullAddress_ { full }
	, Urls_ { full }
	, PreviousDateTime_ { AvatarsTimestampStorage {}.GetTimestamp (full).get_value_or ({}) }
	{
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (refetch ()));
		Timer_->setInterval (120 * 60 * 1000);
		Timer_->start ();


		QTimer::singleShot (2000,
				this,
				SLOT (refetch ()));
	}

	bool SelfAvatarFetcher::IsValid () const
	{
		return Urls_.SmallUrl_.isValid ();
	}

	void SelfAvatarFetcher::refetch ()
	{
		const auto reply = NAM_->head (QNetworkRequest (Urls_.SmallUrl_));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleHeadFinished ()));
	}

	void SelfAvatarFetcher::handleHeadFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();
		if (reply->error () == QNetworkReply::ContentNotFoundError)
		{
			qDebug () << Q_FUNC_INFO
					<< "avatar not found for"
					<< FullAddress_;
			return;
		}

		const auto& dt = reply->header (QNetworkRequest::LastModifiedHeader).toDateTime ();
		if (dt <= PreviousDateTime_)
			return;

		PreviousDateTime_ = dt;

		AvatarsTimestampStorage {}.SetTimestamp (FullAddress_, dt);

		emit avatarChanged ();
	}
}
}
}
