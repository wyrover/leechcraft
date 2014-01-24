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

#include "linuxbackend.h"
#include <cmath>
#include <QFile>
#include <QtDebug>

namespace LeechCraft
{
namespace CpuLoad
{
	LinuxBackend::LinuxBackend (QObject *parent)
	: QObject (parent)
	{
	}

	void LinuxBackend::Update ()
	{
		const int prevCpuCount = GetCpuCount ();
		LastLoads_.clear ();

		QFile file { "/proc/stat" };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		static const QByteArray cpuMarker { "cpu" };

		for (const auto& line : file.readAll ().split ('\n'))
		{
			const auto& elems = line.split (' ');
			const auto& id = elems.value (0);
			if (!id.startsWith (cpuMarker))
				continue;

			bool ok = true;
			const auto cpuIdx = id == cpuMarker ?
					0 :
					(id.mid (cpuMarker.size ()).toInt (&ok) + 1);
			if (!ok)
				continue;

			const auto total = elems.value (1).toLong () +
					elems.value (2).toLong () +
					elems.value (3).toLong () +
					elems.value (4).toLong ();

			auto makeInfo = [&elems, total] (int idx, LoadPriority prio, const QString& label) -> LoadTypeInfo
			{
				return
				{
					label,
					prio,
					static_cast<double> (elems.value (idx).toLong ()) / total
				};
			};

			QMap<LoadPriority, LoadTypeInfo> cpuMap;
			auto insertInfo = [&cpuMap, makeInfo] (int idx, LoadPriority prio, const QString& label)
			{
				cpuMap [prio] = makeInfo (idx, prio, label);
			};

			insertInfo (1, LoadPriority::Medium, tr ("user"));
			insertInfo (2, LoadPriority::Low, tr ("nice"));
			insertInfo (3, LoadPriority::High, tr ("sys"));
			insertInfo (5, LoadPriority::IO, tr ("IO"));

			if (LastLoads_.size () <= cpuIdx)
				LastLoads_.resize (cpuIdx + 1);
			LastLoads_ [cpuIdx] = cpuMap;
		}

		const auto curCpuCount = GetCpuCount ();
		if (curCpuCount != prevCpuCount)
			emit cpuCountChanged (curCpuCount);
	}

	int LinuxBackend::GetCpuCount () const
	{
		return LastLoads_.size () - 1;
	}

	QMap<LoadPriority, LoadTypeInfo> LinuxBackend::GetLoads (int cpu) const
	{
		return LastLoads_.value (cpu + 1);
	}

}
}
