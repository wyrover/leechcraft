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

#include "hotsensors.h"
#include <QIcon>
#include <QAbstractItemModel>
#include <util/util.h>
#include <util/sys/paths.h>

#ifdef Q_OS_LINUX
#include "lmsensorsbackend.h"
#elif defined (Q_OS_MAC)
#include "macosbackend.h"
#endif

#include "historymanager.h"
#include "plotmanager.h"

namespace LeechCraft
{
namespace HotSensors
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("hotsensors");

#ifdef USE_CPP14
		HistoryMgr_ = std::make_unique<HistoryManager> ();
#else
		HistoryMgr_.reset (new HistoryManager);
#endif

#ifdef Q_OS_LINUX
		SensorsMgr_ = std::make_shared<LmSensorsBackend> ();
#elif defined (Q_OS_MAC)
		SensorsMgr_ = std::make_shared<MacOsBackend> ();
#endif

		if (SensorsMgr_)
			connect (SensorsMgr_.get (),
					SIGNAL (gotReadings (Readings_t)),
					HistoryMgr_.get (),
					SLOT (handleReadings (Readings_t)));

#ifdef USE_CPP14
		PlotMgr_ = std::make_unique<PlotManager> (proxy);
#else
		PlotMgr_.reset (new PlotManager { proxy });
#endif
		connect (HistoryMgr_.get (),
				SIGNAL (historyChanged (ReadingsHistory_t)),
				PlotMgr_.get (),
				SLOT (handleHistoryUpdated (ReadingsHistory_t)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HotSensors";
	}

	void Plugin::Release ()
	{
		SensorsMgr_.reset ();
	}

	QString Plugin::GetName () const
	{
		return "HotSensors";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Temperature sensors information quark.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QuarkComponents_t Plugin::GetComponents () const
	{
		auto component = std::make_shared<QuarkComponent> ("hotsensors", "HSQuark.qml");
		component->ContextProps_.push_back ({ "HS_plotManager", PlotMgr_->CreateContextWrapper () });
		return { component };
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotsensors, LeechCraft::HotSensors::Plugin);
