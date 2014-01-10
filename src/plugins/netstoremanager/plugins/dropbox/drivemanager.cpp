/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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

#include "drivemanager.h"
#include <QNetworkRequest>
#include <QtDebug>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/util.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include "account.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace DBox
{
	DriveManager::DriveManager (Account *acc, QObject *parent)
	: QObject (parent)
	, DirectoryId_ ("application/vnd.google-apps.folder")
	, Account_ (acc)
	, SecondRequestIfNoItems_ (true)
	{
	}

	void DriveManager::RequestUserId ()
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this] () { RequestAccountInfo (); };
	}

	void DriveManager::RefreshListing (const QByteArray& parentId)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, parentId] () { RequestFiles (parentId); };
	}

	void DriveManager::ShareEntry (const QString& id, ShareType type)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, type] () { RequestSharingEntry (id, type); };
	}

	void DriveManager::CreateDirectory (const QString& name, const QString& parentId)
	{
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, name, parentId] () { RequestCreateDirectory (name, parentId); };
	}

	void DriveManager::RemoveEntry (const QByteArray& id)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id] () { RequestEntryRemoving (id); };
	}

	void DriveManager::Copy (const QByteArray& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, parentId] () { RequestCopyItem (id, parentId); };
	}

	void DriveManager::Move (const QByteArray& id, const QString& parentId)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, parentId] () { RequestMoveItem (id, parentId); };
	}

	void DriveManager::Upload (const QString& filePath, const QStringList& parentId)
	{
		QString parent = parentId.value (0);
		auto guard = MakeRunnerGuard ();

		if (QFileInfo (filePath).size () < 150 * 1024 * 1024)
			ApiCallQueue_ << [this, filePath, parent] () { RequestUpload (filePath, parent); };
		else
			;//TODO chunk_upload
	}

	void DriveManager::Download (const QString& id, const QString& filepath,
			TaskParameters tp, bool silent, bool open)
	{
		if (id.isEmpty ())
			return;
		auto guard = MakeRunnerGuard ();
		ApiCallQueue_ << [this, id, filepath, tp, silent, open] ()
				{ DownloadFile (id, filepath, tp, silent, open); };
	}

	std::shared_ptr< void > DriveManager::MakeRunnerGuard ()
	{
		const bool shouldRun = ApiCallQueue_.isEmpty ();
		return std::shared_ptr<void> (nullptr, [this, shouldRun] (void*)
			{
				if (shouldRun)
					ApiCallQueue_.dequeue () ();
			});
	}

	void DriveManager::RequestAccountInfo ()
	{
		if (Account_->GetAccessToken ().isEmpty ())
			return;

		QString str = QString ("https://api.dropbox.com/1/account/info?access_token=%1")
				.arg (Account_->GetAccessToken ());
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotAccountInfo ()));
	}

	void DriveManager::RequestFiles (const QByteArray& parentId)
	{
		if (Account_->GetAccessToken ().isEmpty ())
			return;

		QString str = QString ("https://api.dropbox.com/1/metadata/dropbox?access_token=%1&path=%2")
				.arg (Account_->GetAccessToken ())
				.arg (parentId.isEmpty () ? "/" : QString::fromUtf8 (parentId));
		QNetworkRequest request (str);

		request.setHeader (QNetworkRequest::ContentTypeHeader,
				"application/x-www-form-urlencoded");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->get (request);

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotFiles ()));
	}

	void DriveManager::RequestSharingEntry (const QString& id, ShareType type)
	{

		QString str;
		switch (type)
		{
		case ShareType::Preview:
			str = QString ("https://api.dropbox.com/1/media/dropbox/%1?access_token=%2")
					.arg (id)
					.arg (Account_->GetAccessToken ());
			break;
		case ShareType::Share:
			str = QString ("https://api.dropbox.com/1/shares/dropbox/%1?access_token=%2")
					.arg (id)
					.arg (Account_->GetAccessToken ());
			break;
		}

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestFileSharing ()));
	}

	void DriveManager::RequestCreateDirectory (const QString& name, const QString& parentId)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/create_folder?access_token=%1&root=%2&path=%3")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (parentId + "/" + name);

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCreateDirectory ()));
	}

	void DriveManager::RequestEntryRemoving (const QString& id)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/delete?access_token=%1&root=%2&path=%3")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id);
		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->post (request, QByteArray ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestEntryRemoving ()));
	}

	void DriveManager::RequestCopyItem (const QString& id, const QString& parentId)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/copy?access_token=%1&root=%2&from_path=%3&to_path=%4")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id)
				.arg (parentId + "/" + QFileInfo (id).fileName ());

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		Reply2Id_ [reply] = parentId;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCopyItem ()));
	}

	void DriveManager::RequestMoveItem (const QString& id, const QString& parentId)
	{
		QString str = QString ("https://api.dropbox.com/1/fileops/move?access_token=%1&root=%2&from_path=%3&to_path=%4")
				.arg (Account_->GetAccessToken ())
				.arg ("dropbox")
				.arg (id)
				.arg (parentId + "/" + QFileInfo (id).fileName ());

		QNetworkRequest request (str);
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");
		QNetworkReply *reply = Core::Instance ().GetProxy ()->GetNetworkAccessManager ()->
				post (request, QByteArray ());
		Reply2Id_ [reply] = parentId;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleMoveItem ()));
	}

	void DriveManager::RequestUpload (const QString& filePath, const QString& parent)
	{
		emit uploadStatusChanged (tr ("Uploading..."), filePath);

		QFileInfo info (filePath);
		QFile *file = new QFile (filePath);
		if (!file->open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file: "
					<< file->errorString ();
			return;
		}

		const QUrl url (QString ("https://api-content.dropbox.com/1/files_put/%1/%2?access_token=%3")
				.arg ("dropbox")
				.arg (parent + "/" + info.fileName ())
				.arg (Account_->GetAccessToken ()));
		QNetworkRequest request (url);
		request.setPriority (QNetworkRequest::LowPriority);
		request.setHeader (QNetworkRequest::ContentLengthHeader, info.size ());
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/json");

		QNetworkReply *reply = Core::Instance ().GetProxy ()->
				GetNetworkAccessManager ()->put (request, file);
		file->setParent (reply);
		Reply2FilePath_ [reply] = filePath;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleUploadFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleUploadError (QNetworkReply::NetworkError)));
		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
	}

	void DriveManager::DownloadFile (const QString& id, const QString& filePath,
			TaskParameters tp, bool silent, bool open)
	{
		QString savePath;
		if (silent)
			savePath = QDesktopServices::storageLocation (QDesktopServices::TempLocation) +
					"/" + QFileInfo (filePath).fileName ();

		QUrl url (QString ("https://api-content.dropbox.com/1/files/%1/%2?access_token=%3")
				.arg ("dropbox")
				.arg (id)
				.arg (Account_->GetAccessToken ()));
		auto e = Util::MakeEntity (url, savePath, tp);
		QFileInfo fi (filePath);
		e.Additional_ ["Filename"] = QString ("%1_%2.%3")
				.arg (fi.baseName ())
				.arg (QDateTime::currentDateTime ().toTime_t ())
				.arg (fi.completeSuffix ());
		silent ?
			Core::Instance ().DelegateEntity (e, filePath, open) :
			Core::Instance ().SendEntity (e);
	}

	void DriveManager::ParseError (const QVariantMap& map)
	{
		QString msg = map ["error"].toString ();
		Core::Instance ().SendEntity (Util::MakeNotification ("NetStoreManager",
				msg,
				PWarning_));
	}

	void DriveManager::handleGotAccountInfo ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		Account_->SetUserID (res.toMap () ["uid"].toString ());
	}

	namespace
	{
		DBoxItem CreateDBoxItem (const QVariant& itemData)
		{
			const QVariantMap& map = itemData.toMap ();

			DBoxItem driveItem;
			driveItem.FileSize_ = map ["bytes"].toULongLong ();
			driveItem.FolderHash_ = map ["hash"].toString ();
			driveItem.Revision_ = map ["rev"].toByteArray ();
			const auto& path = map ["path"].toString ();
			driveItem.Id_ = path;
			driveItem.ParentID_ = QFileInfo (path).dir ().absolutePath ();
			driveItem.IsDeleted_ = map ["is_deleted"].toBool ();
			driveItem.IsFolder_ = map ["is_dir"].toBool ();
			driveItem.ModifiedDate_ = map ["modified"].toDateTime ();
			driveItem.Name_ = QFileInfo (path).fileName ();
			driveItem.MimeType_ = map ["mime_type"].toString ().replace ('/', '-');

			return driveItem;
		}
	}

	void DriveManager::handleGotFiles ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO << "parse error";
			return;
		}

		const auto& resMap = res.toMap ();
		if (!resMap.contains ("contents"))
		{
			qDebug () << Q_FUNC_INFO << "there are no items";
			if (SecondRequestIfNoItems_)
			{
				SecondRequestIfNoItems_ = false;
				RefreshListing ();
			}
			return;
		}

		SecondRequestIfNoItems_ = true;
		QList<DBoxItem> resList;
		Q_FOREACH (const auto& item, resMap ["contents"].toList ())
		{
			const auto& driveItem = CreateDBoxItem (item);

			if (driveItem.Name_.isEmpty ())
				continue;
			resList << driveItem;
		}

		emit gotFiles (resList);
	}

	void DriveManager::handleRequestFileSharing ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);

		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const auto& map = res.toMap ();
		qDebug () << Q_FUNC_INFO
				<< "file shared successfully";
		emit gotSharedFileUrl (map ["url"].toUrl (), map ["expires"].toDateTime ());
	}

	void DriveManager::handleCreateDirectory ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "directory created successfully";
		emit gotNewItem (CreateDBoxItem (res.toMap ()));
	}

	void DriveManager::handleRequestEntryRemoving ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "file removed successfully";
		emit gotNewItem (CreateDBoxItem (res));
	}

	void DriveManager::handleCopyItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "entry copied successfully";
		RefreshListing (Reply2Id_.take (reply).toUtf8 ());
	}

	void DriveManager::handleMoveItem ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}
		qDebug () << Q_FUNC_INFO
				<< "entry moved successfully";
		RefreshListing (Reply2Id_.take (reply).toUtf8 ());
	}

	void DriveManager::handleUploadFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();

		bool ok = false;
		const auto& res = QJson::Parser ().parse (reply->readAll (), &ok);
		if (!ok)
		{
			qDebug () << Q_FUNC_INFO
					<< "parse error";
			return;
		}

		const auto& map = res.toMap ();
		const auto& id = map ["id"].toString ();

		if (!map.contains ("error"))
		{
			qDebug () << Q_FUNC_INFO
					<< "file uploaded successfully";
			emit gotNewItem (CreateDBoxItem (res));
			emit finished (id, Reply2FilePath_.take (reply));
			return;
		}

		 ParseError (map);
	}

	void DriveManager::handleUploadProgress (qint64 uploaded, qint64 total)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		emit uploadProgress (uploaded, total, Reply2FilePath_ [reply]);
	}

	void DriveManager::handleUploadError (QNetworkReply::NetworkError error)
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		emit uploadError ("Error", Reply2FilePath_.take (reply));
	}
}
}
}

