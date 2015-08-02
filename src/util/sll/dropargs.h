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

#include "typelist.h"
#include "oldcppkludges.h"

namespace LeechCraft
{
namespace Util
{
	namespace detail
	{
		template<typename F, template<typename...> class List, typename... Args>
		constexpr List<Args...> GetInvokablePartImpl (int, List<Args...>, typename std::result_of<F (Args...)>::type* = nullptr)
		{
			return {};
		}

		template<typename F, template<typename...> class List>
		constexpr Typelist<> GetInvokablePartImpl (float, List<>)
		{
			return {};
		}

		template<typename F, typename List>
		struct InvokableType;

		template<typename F, template<typename...> class List, typename... Args>
		constexpr auto GetInvokablePartImpl (float, List<Args...> list) -> typename InvokableType<F, decltype (Reverse (Tail (Reverse (list))))>::RetType_t
		{
			return GetInvokablePartImpl<F> (0, Reverse (Tail (Reverse (list))));
		}

		template<typename F, template<typename...> class List, typename...Args>
		struct InvokableType<F, List<Args...>>
		{
			using RetType_t = decltype (GetInvokablePartImpl<F> (0, List<Args...> {}));
		};

		template<typename F, typename... Args>
		constexpr auto GetInvokablePart () -> decltype (GetInvokablePartImpl<F> (0, Typelist<Args...> {}))
		{
			return GetInvokablePartImpl<F> (0, Typelist<Args...> {});
		}

		template<template<typename...> class List, typename... Args>
		constexpr size_t Length (List<Args...>)
		{
			return sizeof... (Args);
		}

		template<typename T>
		struct Dumbifier
		{
			using Type_t = T;
		};

		template<typename T>
		using Dumbify = typename Dumbifier<T>::Type_t;

		template<typename F, typename List>
		struct InvokableResGetter;

		template<typename F, template<typename...> class List, typename... Args>
		struct InvokableResGetter<F, List<Args...>>
		{
			using RetType_t = ResultOf_t<F (Args...)>;
		};

		template<typename F>
		class Dropper
		{
			F F_;
		public:
			Dropper (const F& f)
			: F_ { f }
			{
			}

			template<typename... Args>
			auto operator() (Args... args) -> typename InvokableResGetter<F, decltype (GetInvokablePart<F, Args...> ())>::RetType_t
			{
				auto invokableList = GetInvokablePart<F, Args...> ();
				auto ignoreList = Drop<Length (decltype (invokableList) {})> (Typelist<Args...> {});
				return Invoke (invokableList, ignoreList, args...);
			}
		private:
			template<typename... InvokableArgs, typename... Rest>
			auto Invoke (Typelist<InvokableArgs...>, Typelist<Rest...>, Dumbify<InvokableArgs>... args, Dumbify<Rest>...) -> ResultOf_t<F (InvokableArgs...)>
			{
				return F_ (args...);
			}
		};
	}

	template<typename F>
	detail::Dropper<F> DropArgs (const F& f)
	{
		return detail::Dropper<F> { f };
	}
}
}
