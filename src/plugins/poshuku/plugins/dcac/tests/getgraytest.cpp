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

#include "getgraytest.h"
#include <QtTest>
#include "../effectsimpl.cpp"

QTEST_APPLESS_MAIN (LeechCraft::Poshuku::DCAC::GetGrayTest)

namespace LeechCraft
{
namespace Poshuku
{
namespace DCAC
{
	void GetGrayTest::testSSE4 ()
	{
		if (!Util::CpuFeatures {}.HasFeature (Util::CpuFeatures::Feature::SSE41))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot run SSE4 test";
			return;
		}

		for (const auto& image : TestImages_)
		{
			const auto ref = GetGrayDefault (image);
			const auto sse4 = GetGraySSE4 (image);

			QCOMPARE (ref, sse4);
		}
	}

	void GetGrayTest::testAVX2 ()
	{
		if (!Util::CpuFeatures {}.HasFeature (Util::CpuFeatures::Feature::AVX2))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot run AVX2 test";
			return;
		}

		for (const auto& image : TestImages_)
		{
			const auto ref = GetGrayDefault (image);
			const auto avx2 = GetGrayAVX2 (image);

			QCOMPARE (ref, avx2);
		}
	}

	void GetGrayTest::benchDefault ()
	{
		BenchmarkFunction (&GetGrayDefault);
	}

	void GetGrayTest::benchSSE4 ()
	{
		if (!Util::CpuFeatures {}.HasFeature (Util::CpuFeatures::Feature::SSE41))
			return;

		BenchmarkFunction (&GetGraySSE4);
	}

	void GetGrayTest::benchAVX2 ()
	{
		if (!Util::CpuFeatures {}.HasFeature (Util::CpuFeatures::Feature::AVX2))
			return;

		BenchmarkFunction (&GetGrayAVX2);
	}
}
}
}
