/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2013  Vladislav Tyulbashev
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

#include <vlc/vlc.h>
#include <QModelIndex>
#include <QVariant>
#include <QTime>
#include <QTimer>
#include <QMimeData>
#include <QDebug>
#include <QItemSelectionModel>
#include <QUrl>
#include "playlistmodel.h"
#include "playlistwidget.h"

namespace LeechCraft
{
namespace vlc
{
	PlaylistModel::PlaylistModel (PlaylistWidget *parent, libvlc_media_list_t *playlist)
	{
		Parent_ = parent;
		Playlist_ = playlist;
		setColumnCount (2);
		setHorizontalHeaderLabels ( {tr ("Name"), tr ("Duration")} );
		setSupportedDragActions (Qt::MoveAction | Qt::CopyAction);
	}
	
	PlaylistModel::~PlaylistModel ()
	{
		for (int i = 0; i < Items_ [0].size (); i++)
			libvlc_media_release (libvlc_media_list_item_at_index (Playlist_, i));
		
		setRowCount (0);
	}
	
	void PlaylistModel::updateTable ()
	{
		setRowCount (libvlc_media_list_count (Playlist_));
		if (libvlc_media_list_count (Playlist_) != Items_ [0].size ())
		{
			int cnt = Items_ [0].size ();
			Items_ [0].resize (libvlc_media_list_count (Playlist_));
			Items_ [1].resize (libvlc_media_list_count (Playlist_));
			
			for (int i = cnt; i < Items_ [0].size (); i++)
			{
				Items_ [0][i] = new QStandardItem;
				Items_ [1][i] = new QStandardItem;
			}
		}
		
		for (int i = 0; i < libvlc_media_list_count (Playlist_); i++)
		{
			libvlc_media_t *media = libvlc_media_list_item_at_index (Playlist_, i);
			Items_ [0][i]->setText (QString::fromUtf8 (libvlc_media_get_meta (media, libvlc_meta_Title)));
			
			if (!libvlc_media_is_parsed (media))
				libvlc_media_parse (media);
				
			QTime time (0, 0);
			time = time.addMSecs (libvlc_media_get_duration (media));
			Items_ [1][i]->setText (time.toString ("hh:mm:ss"));
		}
		
		for (int i = 0; i < libvlc_media_list_count (Playlist_); i++)
		{
			setItem (i, 0, Items_ [0][i]);
			setItem (i, 1, Items_ [1][i]);
		}
	}
	
	bool PlaylistModel::dropMimeData (const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
	{
		QList <QUrl> urls = data->urls ();
		qDebug () << urls;
		if (parent != invisibleRootItem ()->index ())
			row = parent.row () - 1;
		else	
			row -= 2;
		
		
		if (data->colorData ().toString () == "vtyulb")
		{	
			QUrl insertAfter;
			for (int i = row; i > 0; i--)
				if (!urls.contains (QUrl (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL))))
				{
					insertAfter = QUrl (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL));
					break;
				}
			
			QList<libvlc_media_t*> mediaList;
			for (int i = 0; i < urls.size (); i++)
				mediaList.push_back (findAndDelete (urls [i]));
			
			int after;
			if (insertAfter.toString () == "")
				after = -1;
			else
				for (int i = 0; i < libvlc_media_list_count (Playlist_); i++)
					if (QUrl (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL)) == insertAfter)
					{
						after = i;
						break;
					}
					
			if ((parent == QModelIndex ()) && (after == -1))  // VLC forever
			{
				for (int i = 0; i < urls.size (); i++)
					libvlc_media_list_add_media (Playlist_, mediaList [i]);
			}
			else
				for (int i = 0; i < urls.size (); i++)
					libvlc_media_list_insert_media (Playlist_, mediaList [i], after + i + 2);
			
			updateTable ();
		}
		else
		{
			for (int i = 0; i < urls.size (); i++)
				AddUrl (urls [i]);
		}
		
		return true;
	}
	
	QStringList PlaylistModel::mimeTypes () const
	{
		return QStringList ("text/uri-list");
	}
	
	QMimeData* PlaylistModel::mimeData (const QModelIndexList& indexes) const
	{
		if (libvlc_media_list_count (Playlist_) == 1)
			return nullptr;
		
		QMimeData *result = new QMimeData;
		QList<QUrl> urls;
		for (int i = 0; i < indexes.size (); i++)
			if (indexes [i].column () == 0)
			{
				qDebug () << "mimedata" << indexes[i].column() << indexes[i].row();
				urls.push_back (QUrl (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, indexes[i].row ()), libvlc_meta_URL)));
			}
		
		result->setUrls (urls);
		result->setColorData (QVariant ("vtyulb"));
		qDebug () << result->urls ();
		qDebug () << libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, 1), libvlc_meta_URL);
		return result;
	}
	
	Qt::DropActions PlaylistModel::supportedDropActions () const
	{
		return Qt::MoveAction | Qt::CopyAction;
	}
	
	Qt::ItemFlags PlaylistModel::flags (const QModelIndex &index) const
	{
		Qt::ItemFlags defaultFlags = QStandardItemModel::flags (index);
	
		if (index.isValid ())
			return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
		else
			return Qt::ItemIsDropEnabled | defaultFlags;
	}
	
	libvlc_media_t* PlaylistModel::findAndDelete (QUrl url)
	{
		libvlc_media_t *res = nullptr;
		for (int i = 0; i < libvlc_media_list_count (Playlist_); i++)
			if (QUrl (libvlc_media_get_meta (libvlc_media_list_item_at_index (Playlist_, i), libvlc_meta_URL)) == url)
			{
				res = libvlc_media_list_item_at_index (Playlist_, i);
				libvlc_media_list_remove_index (Playlist_, i);
				break;
			}
			
		if (res == nullptr)
			qWarning () << Q_FUNC_INFO << "fatal";
		
		return res;
	}
	
	void PlaylistModel::AddUrl (const QUrl& url)
	{
		Parent_->AddUrl (url);
	}
}
}
