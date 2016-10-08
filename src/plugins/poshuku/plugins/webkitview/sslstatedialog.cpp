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

#include "sslstatedialog.h"
#include <QDateTime>
#include <util/sys/extensionsdata.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/network/sslerror2treeitem.h>
#include <interfaces/core/iiconthememanager.h>
#include "webpagesslwatcher.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace WebKitView
{
	SslStateDialog::SslStateDialog (const WebPageSslWatcher *watcher,
			IIconThemeManager *itm, QWidget *parent)
	: QDialog { parent }
	{
		Ui_.setupUi (this);

		QString iconName;
		QString title;
		switch (watcher->GetPageState ())
		{
		case WebPageSslWatcher::State::NoSsl:
			title = tr ("SSL encryption is not used.");
			return;
		case WebPageSslWatcher::State::SslErrors:
			iconName = "security-low";
			title = tr ("Some SSL errors where encountered.");
			break;
		case WebPageSslWatcher::State::UnencryptedElems:
			iconName = "security-medium";
			title = tr ("Some elements were loaded via unencrypted connection.");
			break;
		case WebPageSslWatcher::State::FullSsl:
			iconName = "security-high";
			title = tr ("Everything is secure!");
			break;
		}

		FillNonSsl (watcher->GetNonSslUrls ());
		FillErrors (watcher->GetErrSslUrls ());

		if (!iconName.isEmpty ())
		{
			const auto& icon = itm->GetIcon (iconName);
			Ui_.IconLabel_->setPixmap (icon.pixmap (16, 16));
		}

		Ui_.OverallStateLabel_->setText (title);

		const auto& config = watcher->GetPageConfiguration ();
		Certs_ = config.peerCertificateChain ();

		for (const auto& cert : Certs_)
		{
			auto name = cert.subjectInfo (QSslCertificate::CommonName);
			if (name.isEmpty ())
				name = cert.subjectInfo (QSslCertificate::Organization);
#if QT_VERSION < 0x050000
			Ui_.CertChainBox_->addItem (name);
#else
			Ui_.CertChainBox_->addItem (name.join ("; "));
#endif
		}
	}

	namespace
	{
		QTreeWidgetItem* MakeUrlItem (const QUrl& url)
		{
			auto item = new QTreeWidgetItem { { url.toString () } };

			const auto& urlExt = url.path ().section ('.', -1, -1);
			item->setIcon (0, Util::ExtensionsData::Instance ().GetExtIcon (urlExt));

			return item;
		}
	}

	void SslStateDialog::FillNonSsl (const QList<QUrl>& nonSsl)
	{
		if (nonSsl.isEmpty ())
		{
			const auto insecureIdx = Ui_.TabWidget_->indexOf (Ui_.InsecureTab_);
			Ui_.TabWidget_->removeTab (insecureIdx);
			return;
		}

		for (const auto& url : nonSsl)
			Ui_.InsecureList_->addTopLevelItem (MakeUrlItem (url));
	}

	void SslStateDialog::FillErrors (const QMap<QUrl, QList<QSslError>>& errors)
	{
		if (errors.isEmpty ())
		{
			const auto errorsIdx = Ui_.TabWidget_->indexOf (Ui_.ErrorsTab_);
			Ui_.TabWidget_->removeTab (errorsIdx);
			return;
		}

		for (const auto& pair : Util::Stlize (errors))
		{
			const auto urlItem = MakeUrlItem (pair.first);
			urlItem->setText (1, {});
			urlItem->addChildren (Util::Map (pair.second, &Util::SslError2TreeItem));
			Ui_.ErrorsList_->addTopLevelItem (urlItem);

			urlItem->setFirstColumnSpanned (true);
		}

		Ui_.ErrorsList_->expandAll ();
		Ui_.ErrorsList_->resizeColumnToContents (0);
		Ui_.ErrorsList_->resizeColumnToContents (1);
	}

	void SslStateDialog::on_CertChainBox__currentIndexChanged (int index)
	{
		Ui_.SslInfoWidget_->SetCertificate (Certs_.value (index));
	}
}
}
}
