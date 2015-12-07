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

#include "nickservidentifywidget.h"
#include <QStandardItemModel>
#include <util/sll/delayedexecutor.h>
#include "acetamide.h"
#include "core.h"
#include "newnickservidentifydialog.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	NickServIdentifyWidget::NickServIdentifyWidget (QStandardItemModel *model,
			QWidget *parent , Qt::WindowFlags f)
	: QWidget (parent, f)
	, Model_ (model)
	{
		Ui_.setupUi (this);

		Ui_.NickServIdentifyView_->setModel (Model_);
		Model_->setHorizontalHeaderLabels ({
				tr ("Server"),
				tr ("Nickname"),
				tr ("NickServ nickname"),
				tr ("NickServ auth string"),
				tr ("Auth message")
			});
		Ui_.NickServIdentifyView_->horizontalHeader ()->setStretchLastSection (true);

		Util::ExecuteLater ([this] { ReadSettings (); });
	}

	void NickServIdentifyWidget::ReadSettings ()
	{
		const auto& list = XmlSettingsManager::Instance ()
				.property ("NickServIdentify").value<QList<QStringList>> ();

		for (const auto& subList : list)
		{
			if (subList.isEmpty ())
				continue;

			NickServIdentify nsi;
			nsi.Server_ = subList [0];
			nsi.Nick_ = subList [1];
			nsi.NickServNick_ = subList [2];
			nsi.AuthString_ = subList [3];
			nsi.AuthMessage_ = subList [4];

			QList<QStandardItem*> identify;
			QStandardItem *serverItem = new QStandardItem (nsi.Server_);
			serverItem->setEditable (false);
			QStandardItem *nickItem = new QStandardItem (nsi.Nick_);
			nickItem->setEditable (false);
			QStandardItem *nickServNickItem = new QStandardItem (nsi.NickServNick_);
			nickServNickItem->setEditable (false);
			QStandardItem *authStringItem = new QStandardItem (nsi.AuthString_);
			authStringItem->setEditable (false);
			QStandardItem *authMessageItem = new QStandardItem (nsi.AuthMessage_);
			authMessageItem->setEditable (false);
			identify << serverItem
					<< nickItem
					<< nickServNickItem
					<< authStringItem
					<< authMessageItem;
			Model_->appendRow (identify);
			Core::Instance ().AddNickServIdentify (nsi);
		}
	}

	void NickServIdentifyWidget::accept ()
	{
		QList<QStringList> list;
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			QStringList record;
			record << Model_->item (i, Column::ServerName)->text ()
					<< Model_->item (i, Column::Nick)->text ()
					<< Model_->item (i, Column::NickServ)->text ()
					<< Model_->item (i, Column::AuthString)->text ()
					<< Model_->item (i, Column::AuthMessage)->text ();
			list << record;
		}

		XmlSettingsManager::Instance ().setProperty ("NickServIdentify", QVariant::fromValue (list));
	}

	void NickServIdentifyWidget::on_Add__clicked ()
	{
		NewNickServIdentifyDialog nns { 0 };
		if (nns.exec () == QDialog::Rejected)
			return;

		QList<QStandardItem*> identify;
		QStandardItem *serverItem = new QStandardItem (nns.GetServer ());
		serverItem->setEditable (false);
		QStandardItem *nickItem = new QStandardItem (nns.GetNickName ());
		nickItem->setEditable (false);
		QStandardItem *nickServNickItem = new QStandardItem (nns.GetNickServNickName ());
		nickServNickItem->setEditable (false);
		QStandardItem *authStringItem = new QStandardItem (nns.GetAuthString ());
		authStringItem->setEditable (false);
		QStandardItem *authMessageItem = new QStandardItem (nns.GetAuthMessage ());
		authMessageItem->setEditable (false);
		NickServIdentify nsi;
		nsi.Server_ = nns.GetServer ();
		nsi.Nick_ = nns.GetNickName ();
		nsi.NickServNick_ = nns.GetNickServNickName ();
		nsi.AuthString_ = nns.GetAuthString ();
		nsi.AuthMessage_ = nns.GetAuthMessage ();

		identify << serverItem
				<< nickItem
				<< nickServNickItem
				<< authStringItem
				<< authMessageItem;
		Model_->appendRow (identify);
		Core::Instance ().AddNickServIdentify (nsi);
	}

	void NickServIdentifyWidget::on_Edit__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;

		std::unique_ptr<NewNickServIdentifyDialog> nns (new NewNickServIdentifyDialog (0));
		nns->SetServer (Model_->item (index.row (), Column::ServerName)->text ());
		nns->SetNickName (Model_->item (index.row (), Column::Nick)->text ());
		nns->SetNickServNickName (Model_->item (index.row (), Column::NickServ)->text ());
		nns->SetAuthString (Model_->item (index.row (), Column::AuthString)->text ());
		nns->SetAuthMessage (Model_->item (index.row (), Column::AuthMessage)->text ());

		if (nns->exec () == QDialog::Rejected)
			return;

		Model_->item (index.row (), Column::ServerName)->setText (nns->GetServer ());
		Model_->item (index.row (), Column::Nick)->setText (nns->GetNickName ());
		Model_->item (index.row (), Column::NickServ)->setText (nns->GetNickServNickName ());
		Model_->item (index.row (), Column::AuthString)->setText (nns->GetAuthString ());
		Model_->item (index.row (), Column::AuthMessage)->setText (nns->GetAuthMessage ());
	}

	void NickServIdentifyWidget::on_Delete__clicked ()
	{
		const QModelIndex& index = Ui_.NickServIdentifyView_->currentIndex ();
		if (!index.isValid ())
			return;
		Model_->removeRow (index.row ());
	}

}
}
}
