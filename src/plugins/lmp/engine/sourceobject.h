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
#include <atomic>
#include <type_traits>
#include <QObject>
#include <QStringList>
#include <QMap>
#include <QMutex>
#include <QWaitCondition>
#include "interfaces/lmp/isourceobject.h"
#include "interfaces/lmp/ipath.h"
#include "util/lmp/gstutil.h"
#include "audiosource.h"
#include "pathelement.h"

typedef struct _GstElement GstElement;
typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;
typedef struct _GstBus GstBus;

typedef std::shared_ptr<GstMessage> GstMessage_ptr;

namespace LeechCraft
{
namespace LMP
{
	class AudioSource;
	class Path;
	class MsgPopThread;

	enum class SourceError
	{
		MissingPlugin,
		SourceNotFound,
		CannotOpenSource,
		InvalidSource,
		Other
	};

	enum class Category
	{
		Music,
		Notification
	};

	class HandlerContainerBase : public QObject
	{
		Q_OBJECT
	protected slots:
		virtual void objectDestroyed () = 0;
	};

	template<typename T>
	class HandlerContainer : public HandlerContainerBase
	{
		QMap<QObject*, QList<T>> Dependents_;
	public:
		void AddHandler (const T& handler, QObject *dependent)
		{
			Dependents_ [dependent] << handler;

			connect (dependent,
					SIGNAL (destroyed (QObject*)),
					this,
					SLOT (objectDestroyed ()));
		}

		template<typename Reducer, typename... Args>
		auto operator() (Reducer r, decltype (r (T {} (Args {}...), T {} (Args {}...))) init, Args... args) -> decltype (r (T {} (args...), T {} (args...)))
		{
			for (const auto& sublist : Dependents_)
				for (const auto& item : sublist)
					init = r (init, item (args...));

			return init;
		}

		template<typename... Args>
		auto operator() (Args... args) -> typename std::enable_if<std::is_same<void, decltype (T {} (args...))>::value, void>::type
		{
			for (const auto& sublist : Dependents_)
				for (const auto& item : sublist)
					item (args...);
		}
	private:
		void objectDestroyed ()
		{
			Dependents_.remove (sender ());
		}
	};

	class SourceObject : public QObject
					   , public ISourceObject
	{
		Q_OBJECT

		friend class Path;

		GstElement *Dec_;

		Path *Path_;

		AudioSource CurrentSource_;
		AudioSource NextSource_;

		AudioSource ActualSource_;

		QMutex NextSrcMutex_;
		QWaitCondition NextSrcWC_;

		bool IsSeeking_;

		qint64 LastCurrentTime_;

		uint PrevSoupRank_;

		QMutex BusDrainMutex_;
		QWaitCondition BusDrainWC_;
		bool IsDrainingMsgs_ = false;

		MsgPopThread *PopThread_;
		GstUtil::TagMap_t Metadata_;

		HandlerContainer<SyncHandler_f> SyncHandlers_;
		HandlerContainer<AsyncHandler_f> AsyncHandlers_;
	public:
		enum class Metadata
		{
			Artist,
			Album,
			Title,
			Genre,
			Tracknumber,
			NominalBitrate,
			MinBitrate,
			MaxBitrate
		};
	private:
		SourceState OldState_;
	public:
		SourceObject (Category, QObject* = 0);
		~SourceObject ();

		SourceObject (const SourceObject&) = delete;
		SourceObject& operator= (const SourceObject&) = delete;

		QObject* GetQObject ();

		bool IsSeekable () const;

		SourceState GetState () const;
		void SetState (SourceState);

		QString GetErrorString () const;

		QString GetMetadata (Metadata) const;

		qint64 GetCurrentTime ();
		qint64 GetRemainingTime () const;
		qint64 GetTotalTime () const;
		void Seek (qint64);

		AudioSource GetActualSource () const;
		AudioSource GetCurrentSource () const;
		void SetCurrentSource (const AudioSource&);
		void PrepareNextSource (const AudioSource&);

		void Play ();
		void Pause ();
		void Stop ();

		void Clear ();
		void ClearQueue ();

		void HandleAboutToFinish ();

		void SetupSource ();

		void AddToPath (Path*);
		void SetSink (GstElement*);

		void AddSyncHandler (const SyncHandler_f&, QObject*);
		void AddAsyncHandler (const AsyncHandler_f&, QObject*);
	private:
		void HandleErrorMsg (GstMessage*);
		void HandleTagMsg (GstMessage*);
		void HandleBufferingMsg (GstMessage*);
		void HandleStateChangeMsg (GstMessage*);
		void HandleEosMsg (GstMessage*);
		void HandleStreamStatusMsg (GstMessage*);
		void HandleWarningMsg (GstMessage*);

		int HandleSyncMessage (GstBus*, GstMessage*);
	private slots:
		void handleMessage (GstMessage_ptr);
		void updateTotalTime ();
		void handleTick ();

		void setActualSource (const AudioSource&);
	signals:
		void stateChanged (SourceState, SourceState);
		void currentSourceChanged (const AudioSource&);
		void aboutToFinish (std::shared_ptr<std::atomic_bool>);
		void finished ();
		void metaDataChanged ();
		void bufferStatus (int);
		void totalTimeChanged (qint64);

		void tick (qint64);

		void error (const QString&, SourceError);
	};
}
}
