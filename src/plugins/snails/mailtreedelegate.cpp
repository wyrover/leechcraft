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

#include "mailtreedelegate.h"
#include <QPainter>
#include <QMouseEvent>
#include <QToolBar>
#include <QTreeView>
#include <QProxyStyle>
#include <QMenu>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include <util/sll/delayedexecutor.h>
#include "common.h"
#include "mailtab.h"
#include "mailmodel.h"
#include "messagelistactioninfo.h"

namespace LeechCraft
{
namespace Snails
{
	MailTreeDelegate::MailTreeDelegate (const MessageLoader_f& loader,
			QTreeView *view, QObject *parent)
	: QStyledItemDelegate { parent }
	, Loader_ { loader }
	, View_ { view }
	, Mode_ { MailListMode::Normal }
	{
	}

	const int Padding = 2;

	namespace
	{
		QString GetString (const QModelIndex& index, MailModel::Column column)
		{
			return index.sibling (index.row (), static_cast<int> (column)).data ().toString ();
		}

		QPair<QFont, QFontMetrics> GetSubjectFont (const QModelIndex& index,
				const QStyleOptionViewItem& option)
		{
			const bool isRead = index.data (MailModel::MailRole::IsRead).toBool ();
			const bool isEnabled = index.flags () & Qt::ItemIsEnabled;

			auto subjectFont = option.font;
			if (!isRead && isEnabled)
				subjectFont.setBold (true);

			return { subjectFont, QFontMetrics { subjectFont } };
		}

		int GetActionsBarWidth (const QModelIndex& index, const QStyleOptionViewItem& option, int subjHeight)
		{
			const auto& acts = index.data (MailModel::MailRole::MessageActions)
					.value<QList<MessageListActionInfo>> ();
			if (acts.isEmpty ())
				return 0;

			const auto style = option.widget ?
					option.widget->style () :
					QApplication::style ();
			const auto spacing = style->pixelMetric (QStyle::PM_ToolBarItemSpacing, &option);

			return acts.size () * subjHeight +
					(acts.size () - 1) * spacing +
					2 * Padding;
		}

		void DrawCheckbox (QPainter *painter, QStyle *style,
				QStyleOptionViewItemV4& option, const QModelIndex& index)
		{
			QStyleOptionButton checkBoxOpt;
			static_cast<QStyleOption&> (checkBoxOpt) = option;
			switch (index.data (Qt::CheckStateRole).value<Qt::CheckState> ())
			{
			case Qt::Checked:
				checkBoxOpt.state |= QStyle::State_On;
				break;
			case Qt::Unchecked:
				checkBoxOpt.state |= QStyle::State_Off;
				break;
			case Qt::PartiallyChecked:
				checkBoxOpt.state |= QStyle::State_NoChange;
				break;
			}
			style->drawControl (QStyle::CE_CheckBox, &checkBoxOpt, painter);

			const auto checkboxWidth = style->pixelMetric (QStyle::PM_IndicatorWidth) +
					style->pixelMetric (QStyle::PM_CheckBoxLabelSpacing);
			option.rect.setLeft (option.rect.left () + checkboxWidth);
		}

		QStyle* GetStyle (const QStyleOptionViewItem& option)
		{
			return option.widget ?
					option.widget->style () :
					QApplication::style ();
		}
	}

	void MailTreeDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& stockItem, const QModelIndex& index) const
	{
		const bool isEnabled = index.flags () & Qt::ItemIsEnabled;

		QStyleOptionViewItemV4 option { stockItem };
		if (!isEnabled)
			option.font.setStrikeOut (true);

		const auto style = GetStyle (option);

		painter->save ();

		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

		if (option.state & QStyle::State_Selected)
			painter->setPen (option.palette.color (QPalette::HighlightedText));

		if (Mode_ == MailListMode::MultiSelect)
			DrawCheckbox (painter, style, option, index);

		const auto& subject = GetString (index, MailModel::Column::Subject);

		const auto& subjFontInfo = GetSubjectFont (index, option);
		const auto subjHeight = subjFontInfo.second.boundingRect (subject).height ();
		auto y = option.rect.top () + subjHeight;

		const auto actionsWidth = GetActionsBarWidth (index, option, subjHeight);

		painter->setFont (subjFontInfo.first);
		painter->drawText (option.rect.left (),
				y,
				subjFontInfo.second.elidedText (subject, Qt::ElideRight,
						option.rect.width () - actionsWidth));

		const QFontMetrics fontFM { option.font };

		auto stringHeight = [&fontFM] (const QString& str)
		{
			return fontFM.boundingRect (str).height ();
		};

		auto from = GetString (index, MailModel::Column::From);
		if (const auto childrenCount = index.data (MailModel::MailRole::TotalChildrenCount).toInt ())
		{
			from += " (";
			if (const auto unread = index.data (MailModel::MailRole::UnreadChildrenCount).toInt ())
				from += QString::number (unread) + "/";
			from += QString::number (childrenCount) + ")";
		}
		const auto& date = GetString (index, MailModel::Column::Date);

		y += std::max ({ stringHeight (from), stringHeight (date) });

		painter->setFont (option.font);

		const auto dateWidth = fontFM.boundingRect (date).width ();
		painter->drawText (option.rect.right () - dateWidth,
				y,
				date);

		painter->drawText (option.rect.left (),
				y,
				fontFM.elidedText (from, Qt::ElideRight, option.rect.width () - dateWidth - 5 * Padding));

		painter->restore ();
	}

	QSize MailTreeDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto& subjFontInfo = GetSubjectFont (index, option);
		const QFontMetrics plainFM { option.font };

		const auto width = View_->viewport ()->width ();
		const auto height = 2 * Padding +
				subjFontInfo.second.boundingRect (GetString (index, MailModel::Column::Subject)).height () +
				plainFM.boundingRect (GetString (index, MailModel::Column::From)).height ();

		return { width, height };
	}

	namespace
	{
		class NullMarginsStyle : public QProxyStyle
		{
		public:
			using QProxyStyle::QProxyStyle;

			int pixelMetric (PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override
			{
				if (metric == QStyle::PM_ToolBarItemMargin || metric == QStyle::PM_ToolBarFrameWidth)
					return 0;

				return QProxyStyle::pixelMetric (metric, option, widget);
			}
		};

		template<typename Loader, typename ContainerT>
		void BuildAction (const Loader& loader, ContainerT *container, const MessageListActionInfo& actInfo)
		{
			const auto action = container->addAction (actInfo.Icon_, actInfo.Name_);
			action->setToolTip (actInfo.Description_);

			if (actInfo.Children_.isEmpty ())
				new Util::SlotClosure<Util::NoDeletePolicy>
				{
					[loader, handler = actInfo.Handler_]
					{
						Message_ptr msg;
						try
						{
							msg = loader ();
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to load message:"
									<< e.what ();
							return;
						}

						handler (msg);
					},
					action,
					SIGNAL (triggered ()),
					action
				};
			else
			{
				auto menu = new QMenu;
				menu->setIcon (actInfo.Icon_);
				menu->setTitle (actInfo.Name_);
				action->setMenu (menu);

				for (const auto& child : actInfo.Children_)
					BuildAction (loader, menu, child);
			}
		}
	}

	QWidget* MailTreeDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto& actionsVar = index.data (MailModel::MailRole::MessageActions);
		if (actionsVar.isNull ())
			return nullptr;

		const auto& actionInfos = actionsVar.value<QList<MessageListActionInfo>> ();
		if (actionInfos.isEmpty ())
			return nullptr;

		const auto& id = index.data (MailModel::MailRole::ID).toByteArray ();

		const auto container = new QToolBar { parent };
		auto style = new NullMarginsStyle;
		style->setParent (container);
		container->setStyle (style);
		for (const auto& actInfo : actionInfos)
			BuildAction (std::bind (Loader_, id), container, actInfo);

		Util::ExecuteLater ([=] { updateEditorGeometry (container, option, index); });

		return container;
	}

	void MailTreeDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto& subjFM = GetSubjectFont (index, option).second;
		auto height = subjFM.boundingRect (GetString (index, MailModel::Column::Subject)).height ();

		qobject_cast<QToolBar*> (editor)->setIconSize ({ height, height });

		editor->setMaximumSize (option.rect.size ());
		editor->move (option.rect.topRight () - QPoint { editor->width (), 0 });
	}

	bool MailTreeDelegate::eventFilter (QObject *object, QEvent *event)
	{
		blockSignals (true);
		const auto res = QStyledItemDelegate::eventFilter (object, event);
		blockSignals (false);
		return res;
	}

	void MailTreeDelegate::setMailListMode (MailListMode mode)
	{
		if (Mode_ == mode)
			return;

		Mode_ = mode;

		auto idx = View_->indexAt (View_->rect ().topLeft ());
		while (idx.isValid ())
		{
			View_->update (idx);
			idx = View_->indexBelow (idx);
		}
	}
}
}
