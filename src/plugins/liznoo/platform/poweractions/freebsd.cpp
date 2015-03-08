/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
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

#include "freebsd.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dev/acpica/acpiio.h>
#include <QMessageBox>

namespace LeechCraft
{
namespace Liznoo
{
namespace PowerActions
{
	class FDGuard
	{
		int FD_;
	public:
		FDGuard (const char *file, int mode)
		: FD_ { open (file, mode) }
		{
		}

		FDGuard (const FDGuard&) = delete;

		FDGuard (FDGuard&& other)
		: FD_ { other.FD_ }
		{
			other.FD_ = -1;
		}

		~FDGuard ()
		{
			if (FD_ >= 0)
				close (FD_);
		}

		explicit operator bool () const
		{
			return FD_ >= 0;
		}

		operator int () const
		{
			return FD_;
		}
	};

	QFuture<Platform::QueryChangeStateResult> FreeBSD::CanChangeState (Platform::State)
	{
		QFutureInterface<QueryChangeStateResult> iface;

		if (FDGuard { "/dev/acpi", O_WRONLY })
		{
			const QueryChangeStateResult result { true, {} };
			iface.reportFinished (&result);
		}
		else
		{
			const auto& msg = errno == EACCES ?
					tr ("No permissions to write to %1. If you are in the %2 group, add "
						"%3 to %4 and run %5 to apply the required permissions to %1.")
						.arg ("<em>/dev/acpi</em>")
						.arg ("<em>wheel</em>")
						.arg ("<code>perm acpi 0664</code>")
						.arg ("<em>/etc/devfs.conf</em>")
						.arg ("<code>/etc/rc.d/devfs restart</code>") :
					tr ("Unable to open %1 for writing.")
						.arg ("<em>/dev/acpi</em>");
			const QueryChangeStateResult result { false, msg };
			iface.reportFinished (&result);
		}

		return iface.future ();
	}

	void FreeBSD::ChangeState (Platform::State state)
	{
		const FDGuard fd { "/dev/acpi", O_WRONLY };
		if (!fd)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open /dev/acpi for writing, errno is:"
					<< errno;
			return;
		}

		int sleep_state = -1;
		switch (state)
		{
		case State::Suspend:
			sleep_state = 3;
			break;
		case State::Hibernate:
			sleep_state = 4;
			break;
		}
		if (fd && sleep_state > 0)
			ioctl (fd, ACPIIO_REQSLPSTATE, &sleep_state); // this requires root privileges by default
	}
}
}
}