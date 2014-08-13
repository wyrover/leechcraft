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

#include <memory>
#include <functional>
#include <QWidget>
#include <QModelIndex>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_photostab.h"

class QComboBox;
class QSlider;

#if QT_VERSION < 0x050000
class QDeclarativeView;
#else
class QQuickWidget;
#endif

namespace LeechCraft
{
namespace Blasq
{
	class AccountsManager;
	class PhotosProxyModel;
	class IAccount;
	class UploadPhotosDialog;

	class PhotosTab : public QWidget
					, public ITabWidget
					, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::PhotosTab Ui_;
#if QT_VERSION < 0x050000
		QDeclarativeView * const ImagesView_;
#else
		QQuickWidget * const ImagesView_;
#endif

		const TabClassInfo TC_;
		QObject * const Plugin_;

		AccountsManager * const AccMgr_;
		const ICoreProxy_ptr Proxy_;

		PhotosProxyModel * const ProxyModel_;

		QComboBox *AccountsBox_;
		QAction *UploadAction_;
		QSlider *UniSlider_;
		std::unique_ptr<QToolBar> Toolbar_;

		IAccount *CurAcc_ = 0;
		QObject *CurAccObj_ = 0;

		QString OnUpdateCollectionId_;

		QString SelectedID_;
		QString SelectedCollection_;

		QStringList SelectedIDsSet_;

		bool SingleImageMode_ = false;
	public:
		PhotosTab (AccountsManager*, const TabClassInfo&, QObject*, ICoreProxy_ptr);
		PhotosTab (AccountsManager*, ICoreProxy_ptr);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		void Remove ();
		QToolBar* GetToolBar () const;

		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;
		QByteArray GetTabRecoverData () const;

		void RecoverState (QDataStream&);

		void SelectAccount (const QByteArray&);

		QModelIndexList GetSelectedImages () const;
	private:
		void AddScaleSlider ();

		void HandleImageSelected (const QModelIndex&);
		void HandleCollectionSelected (const QModelIndex&);

		QModelIndex ImageID2Index (const QString&) const;
		QModelIndexList ImageID2Indexes (const QString&) const;

		QByteArray GetUniSettingName () const;

		void FinishUploadDialog (UploadPhotosDialog&);

		void PerformCtxMenu (std::function<void (QModelIndex)>);
	private slots:
		void handleAccountChosen (int);
		void handleRowChanged (const QModelIndex&);

		void on_CollectionsTree__customContextMenuRequested (const QPoint&);

		void handleScaleSlider (int);

		void uploadPhotos ();
		void handleUploadRequested ();

		void handleImageSelected (const QString&);
		void handleToggleSelectionSet (const QString&);
		void handleImageOpenRequested (const QVariant&);
		void handleImageOpenRequested ();
		void handleImageDownloadRequested (const QVariant&);
		void handleImageDownloadRequested ();
		void handleCopyURLRequested (const QVariant&);
		void handleCopyURLRequested ();
		void handleDeleteRequested (const QString&);
		void handleDeleteRequested ();
		void handleAlbumSelected (const QVariant&);
		void handleSingleImageMode (bool);

		void handleAccDoneUpdating ();
	signals:
		void removeTab (QWidget*);

		void tabRecoverDataChanged ();
	};
}
}
