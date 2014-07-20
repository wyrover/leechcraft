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

#include <boost/range.hpp>

namespace LeechCraft
{
namespace Util
{
	template<typename Iter, typename Assoc>
	class StlAssocIteratorAdaptor : public boost::iterator_adaptor<
				StlAssocIteratorAdaptor<Iter, Assoc>,
				Iter,
				std::pair<typename Assoc::key_type, typename Assoc::mapped_type>,
				boost::use_default,
				std::pair<typename Assoc::key_type, typename Assoc::mapped_type>
			>
	{
		friend class boost::iterator_core_access;

		typedef boost::iterator_adaptor<
					StlAssocIteratorAdaptor<Iter, Assoc>,
					Iter,
					std::pair<typename Assoc::key_type, typename Assoc::mapped_type>,
					boost::use_default,
					std::pair<typename Assoc::key_type, typename Assoc::mapped_type>
				> Super_t;
	public:
		StlAssocIteratorAdaptor () = default;

		StlAssocIteratorAdaptor (const Iter& it)
		: Super_t { it }
		{
		}
	private:
		typename Super_t::reference dereference () const
		{
			return { this->base ().key (), this->base ().value () };
		}
	};

	template<typename Iter, typename Assoc>
	struct StlAssocRange : public boost::iterator_range<StlAssocIteratorAdaptor<Iter, Assoc>>
	{
	public:
		StlAssocRange (const Assoc& assoc)
		: boost::iterator_range<StlAssocIteratorAdaptor<Iter, Assoc>> { assoc.begin (), assoc.end () }
		{
		}
	};

	template<typename Assoc>
	StlAssocRange<typename Assoc::const_iterator, Assoc> Stlize (const Assoc& assoc)
	{
		return { assoc };
	}
}
}
