/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "vkaccount.h"
#include <QUuid>
#include <QStandardItemModel>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QHttpMultiPart>
#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <qjson/parser.h>
#include <util/svcauth/vkauthmanager.h>
#include <util/queuemanager.h>
#include "vkservice.h"
#include "albumsettingsdialog.h"

namespace LeechCraft
{
namespace Blasq
{
namespace Rappor
{
	VkAccount::VkAccount (const QString& name, VkService *service, ICoreProxy_ptr proxy, const QByteArray& id, const QByteArray& cookies)
	: QObject (service)
	, Name_ (name)
	, ID_ (id.isEmpty () ? QUuid::createUuid ().toByteArray () : id)
	, Service_ (service)
	, Proxy_ (proxy)
	, CollectionsModel_ (new NamedModel<QStandardItemModel> (this))
	, AuthMgr_ (new Util::SvcAuth::VkAuthManager ("3762977", { "photos" }, cookies, proxy))
	, RequestQueue_ (new Util::QueueManager (350, this))
	{
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });

		AllPhotosItem_ = new QStandardItem (tr ("All photos"));
		AllPhotosItem_->setData (ItemType::AllPhotos, CollectionRole::Type);
		AllPhotosItem_->setEditable (false);
		CollectionsModel_->appendRow (AllPhotosItem_);

		connect (AuthMgr_,
				SIGNAL (cookiesChanged (QByteArray)),
				this,
				SLOT (handleCookies (QByteArray)));
		connect (AuthMgr_,
				SIGNAL (gotAuthKey (QString)),
				this,
				SLOT (handleAuthKey (QString)));
	}

	QByteArray VkAccount::Serialize () const
	{
		QByteArray result;
		{
			QDataStream out (&result, QIODevice::WriteOnly);
			out << static_cast<quint8> (1)
					<< Name_
					<< ID_
					<< LastCookies_;
		}
		return result;
	}

	VkAccount* VkAccount::Deserialize (const QByteArray& ba, VkService *service, ICoreProxy_ptr proxy)
	{
		QDataStream in (ba);

		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return nullptr;
		}

		QString name;
		QByteArray id;
		QByteArray cookies;
		in >> name >> id >> cookies;

		return new VkAccount (name, service, proxy, id, cookies);
	}

	QObject* VkAccount::GetQObject ()
	{
		return this;
	}

	IService* VkAccount::GetService () const
	{
		return Service_;
	}

	QString VkAccount::GetName () const
	{
		return Name_;
	}

	QByteArray VkAccount::GetID () const
	{
		return ID_;
	}

	QAbstractItemModel* VkAccount::GetCollectionsModel () const
	{
		return CollectionsModel_;
	}

	void VkAccount::UpdateCollections ()
	{
		if (auto rc = AllPhotosItem_->rowCount ())
			AllPhotosItem_->removeRows (0, rc);

		auto rootRc = CollectionsModel_->rowCount ();
		if (rootRc > 1)
			CollectionsModel_->removeRows (1, rootRc - 1);

		Albums_.clear ();

		CallQueue_.append ([this] (const QString& authKey) -> void
			{
				QUrl albumsUrl ("https://api.vk.com/method/photos.getAlbums.xml");
				albumsUrl.addQueryItem ("access_token", authKey);
				RequestQueue_->Schedule ([this, albumsUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (albumsUrl)),
								SIGNAL (finished ()),
								this,
								SLOT (handleGotAlbums ()));
					}, this);

				QUrl photosUrl ("https://api.vk.com/method/photos.getAll.xml");
				photosUrl.addQueryItem ("access_token", authKey);
				photosUrl.addQueryItem ("count", "100");
				photosUrl.addQueryItem ("photo_sizes", "1");
				RequestQueue_->Schedule ([this, photosUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (photosUrl)),
								SIGNAL (finished ()),
								this,
								SLOT (handleGotPhotos ()));
					}, this);
			});

		AuthMgr_->GetAuthKey ();
	}

	bool VkAccount::HasUploadFeature (Feature feature) const
	{
		switch (feature)
		{
		case Feature::RequiresAlbumOnUpload:
			return true;
		}

		return false;
	}

	void VkAccount::CreateCollection (const QModelIndex&)
	{
		AlbumSettingsDialog dia ({}, Proxy_);
		if (dia.exec () != QDialog::Accepted)
			return;

		const struct
		{
			QString Name_;
			QString Desc_;
			int Priv_;
			int CommentPriv_;
		} params
		{
			dia.GetName (),
			dia.GetDesc (),
			dia.GetPrivacyLevel (),
			dia.GetCommentsPrivacyLevel ()
		};

		CallQueue_.append ([this, params] (const QString& authKey) -> void
			{
				QUrl createUrl ("https://api.vk.com/method/photos.createAlbum.xml");
				createUrl.addQueryItem ("title", params.Name_);
				createUrl.addQueryItem ("description", params.Desc_);
				createUrl.addQueryItem ("privacy", QString::number (params.Priv_));
				createUrl.addQueryItem ("comment_privacy", QString::number (params.CommentPriv_));
				createUrl.addQueryItem ("access_token", authKey);
				RequestQueue_->Schedule ([this, createUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (createUrl)),
								SIGNAL (finished ()),
								this,
								SLOT (handleAlbumCreated ()));
					}, this);
			});

		AuthMgr_->GetAuthKey ();
	}

	void VkAccount::UploadImages (const QModelIndex& collection, const QStringList& paths)
	{
		const auto& aidStr = collection.data (CollectionRole::ID).toString ();

		CallQueue_.append ([this, paths, aidStr] (const QString& authKey) -> void
			{
				QUrl getUrl ("https://api.vk.com/method/photos.getUploadServer.xml");
				getUrl.addQueryItem ("aid", aidStr);
				getUrl.addQueryItem ("access_token", authKey);
				RequestQueue_->Schedule ([this, getUrl, paths] () -> void
					{
						auto reply = Proxy_->GetNetworkAccessManager ()->
								get (QNetworkRequest (getUrl));
						connect (reply,
								SIGNAL (finished ()),
								this,
								SLOT (handlePhotosUploadServer ()));
						PhotosUploadServer2Paths_ [reply] = paths;
					}, this);
			});

		AuthMgr_->GetAuthKey ();
	}

	void VkAccount::HandleAlbumElement (const QDomElement& albumElem)
	{
		const auto& title = albumElem.firstChildElement ("title").text ();
		auto item = new QStandardItem (title);
		item->setEditable (false);
		item->setData (ItemType::Collection, CollectionRole::Type);

		const auto& aidStr = albumElem.firstChildElement ("aid").text ();
		item->setData (aidStr, CollectionRole::ID);

		CollectionsModel_->appendRow (item);

		const auto aid = aidStr.toInt ();
		Albums_ [aid] = item;
	}

	bool VkAccount::HandlePhotoElement (const QDomElement& photoElem, bool atEnd)
	{
		auto mkItem = [&photoElem] () -> QStandardItem*
		{
			const auto& idText = photoElem.firstChildElement ("pid").text ();

			auto item = new QStandardItem (idText);
			item->setData (ItemType::Image, CollectionRole::Type);
			item->setData (idText, CollectionRole::ID);
			item->setData (idText, CollectionRole::Name);

			const auto& sizesElem = photoElem.firstChildElement ("sizes");
			auto getType = [&sizesElem] (const QString& type) -> QPair<QUrl, QSize>
			{
				auto sizeElem = sizesElem.firstChildElement ("size");
				while (!sizeElem.isNull ())
				{
					if (sizeElem.firstChildElement ("type").text () != type)
					{
						sizeElem = sizeElem.nextSiblingElement ("size");
						continue;
					}

					const auto& src = sizeElem.firstChildElement ("src").text ();
					const auto width = sizeElem.firstChildElement ("width").text ().toInt ();
					const auto height = sizeElem.firstChildElement ("height").text ().toInt ();

					return { src, { width, height } };
				}

				return {};
			};

			const auto& small = getType ("m");
			const auto& mid = getType ("x");
			auto orig = getType ("w");
			QStringList sizeCandidates { "z", "y", "x", "r" };
			while (orig.second.width () <= 0)
			{
				if (sizeCandidates.isEmpty ())
					return nullptr;

				orig = getType (sizeCandidates.takeFirst ());
			}

			item->setData (small.first, CollectionRole::SmallThumb);
			item->setData (small.second, CollectionRole::SmallThumbSize);

			item->setData (mid.first, CollectionRole::MediumThumb);
			item->setData (mid.second, CollectionRole::MediumThumbSize);

			item->setData (orig.first, CollectionRole::Original);
			item->setData (orig.second, CollectionRole::OriginalSize);

			return item;
		};

		auto allItem = mkItem ();
		if (!allItem)
			return false;

		if (atEnd)
			AllPhotosItem_->appendRow (allItem);
		else
			AllPhotosItem_->insertRow (0, allItem);

		const auto aid = photoElem.firstChildElement ("aid").text ().toInt ();
		if (Albums_.contains (aid))
		{
			auto album = Albums_ [aid];
			if (atEnd)
				album->appendRow (mkItem ());
			else
				album->insertRow (0, mkItem ());
		}

		return true;
	}

	void VkAccount::handleGotAlbums ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		auto albumElem = doc
				.documentElement ()
				.firstChildElement ("album");
		while (!albumElem.isNull ())
		{
			HandleAlbumElement (albumElem);
			albumElem = albumElem.nextSiblingElement ("album");
		}
	}

	void VkAccount::handleAlbumCreated ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		HandleAlbumElement (doc.documentElement ().firstChildElement ("album"));
	}

	void VkAccount::handlePhotosUploadServer ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		const auto& server = doc.documentElement ().firstChildElement ("upload_url").text ();
		const auto& paths = PhotosUploadServer2Paths_.take (reply);

		for (int reqIdx = 0; reqIdx <= paths.size () / 5 + 1; ++reqIdx)
		{
			auto multipart = new QHttpMultiPart (QHttpMultiPart::FormDataType);

			bool added = false;
			for (int fileIdx = 0; fileIdx < 5 && reqIdx * 5 + fileIdx < paths.size (); ++fileIdx)
			{
				const auto& path = paths.at (reqIdx * 5 + fileIdx);

				auto file = new QFile (path, multipart);
				file->open (QIODevice::ReadOnly);

				QHttpPart filePart;

				const auto& disp = QString ("form-data; name=\"file%1\"; filename=\"%2\"")
						.arg (fileIdx + 1)
						.arg (QFileInfo (path).fileName ());
				filePart.setHeader (QNetworkRequest::ContentDispositionHeader, disp);

				filePart.setBodyDevice (file);

				multipart->append (filePart);

				added = true;
			}

			if (!added)
			{
				delete multipart;
				break;
			}

			const auto nam = Proxy_->GetNetworkAccessManager ();
			auto reply = nam->post (QNetworkRequest (QUrl (server)), multipart);
			connect (reply,
					SIGNAL (finished ()),
					this,
					SLOT (handlePhotosUploaded ()));
			connect (reply,
					SIGNAL (uploadProgress (qint64, qint64)),
					this,
					SLOT (handlePhotosUploadProgress (qint64, qint64)));
			multipart->setParent (reply);
		}
	}

	void VkAccount::handlePhotosUploadProgress (qint64 done, qint64 total)
	{
		qDebug () << "upload" << done << total;
	}

	void VkAccount::handlePhotosUploaded ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		const auto& parsed = QJson::Parser ().parse (data).toMap ();

		CallQueue_.append ([this, parsed] (const QString& authKey) -> void
			{
				QUrl saveUrl ("https://api.vk.com/method/photos.save.xml");
				auto add = [&saveUrl, &parsed] (const QString& name)
					{ saveUrl.addQueryItem (name, parsed [name].toString ()); };
				add ("server");
				add ("photos_list");
				add ("aid");
				add ("hash");
				saveUrl.addQueryItem ("access_token", authKey);
				RequestQueue_->Schedule ([this, saveUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (saveUrl)),
								SIGNAL (finished ()),
								this,
								SLOT (handlePhotosSaved ()));
					}, this);
			});
		AuthMgr_->GetAuthKey ();
	}

	void VkAccount::handlePhotosSaved ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		auto photoElem = doc
				.documentElement ()
				.firstChildElement ("photo");
		while (!photoElem.isNull ())
		{
			HandlePhotoElement (photoElem);
			photoElem = photoElem.nextSiblingElement ("photo");
		}
	}

	void VkAccount::handleGotPhotos ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot parse reply"
					<< data;
			return;
		}

		bool finishReached = false;

		auto photoElem = doc
				.documentElement ()
				.firstChildElement ("photo");
		while (!photoElem.isNull ())
		{
			if (!HandlePhotoElement (photoElem))
			{
				finishReached = true;
				break;
			}

			photoElem = photoElem.nextSiblingElement ("photo");
		}

		if (finishReached)
			return;

		const auto count = doc.documentElement ().firstChildElement ("count").text ().toInt ();
		if (count == AllPhotosItem_->rowCount ())
			return;

		const auto offset = AllPhotosItem_->rowCount ();

		CallQueue_.append ([this, offset] (const QString& authKey) -> void
			{
				QUrl photosUrl ("https://api.vk.com/method/photos.getAll.xml");
				photosUrl.addQueryItem ("access_token", authKey);
				photosUrl.addQueryItem ("count", "100");
				photosUrl.addQueryItem ("offset", QString::number (offset));
				photosUrl.addQueryItem ("photo_sizes", "1");
				RequestQueue_->Schedule ([this, photosUrl]
					{
						connect (Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (photosUrl)),
								SIGNAL (finished ()),
								this,
								SLOT (handleGotPhotos ()));
					}, this);
			});

		AuthMgr_->GetAuthKey ();

	}

	void VkAccount::handleAuthKey (const QString& authKey)
	{
		while (!CallQueue_.isEmpty ())
			CallQueue_.takeFirst () (authKey);
	}

	void VkAccount::handleCookies (const QByteArray& cookies)
	{
		LastCookies_ = cookies;
		emit accountChanged (this);
	}
}
}
}
