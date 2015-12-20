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

#include <type_traits>
#include <boost/variant.hpp>
#include "functor.h"
#include "applicative.h"
#include "monad.h"

namespace LeechCraft
{
namespace Util
{
	template<typename L, typename R>
	class Either
	{
		using Either_t = boost::variant<L, R>;
		Either_t This_;

		enum { LeftVal, RightVal };

		static_assert (!std::is_same<L, R>::value, "Types cannot be the same.");
	public:
		Either () = delete;

		explicit Either (const L& l)
		: This_ { l }
		{
		}

		explicit Either (const R& r)
		: This_ { r }
		{
		}

		Either (const Either&) = default;
		Either (Either&&) = default;
		Either& operator= (const Either&) = default;
		Either& operator= (Either&&) = default;

		bool IsLeft () const
		{
			return This_.which () == LeftVal;
		}

		bool IsRight () const
		{
			return This_.which () == RightVal;
		}

		const L& GetLeft () const
		{
			if (!IsLeft ())
				throw std::runtime_error { "Tried accessing Left for a Right Either" };
			return boost::get<L> (This_);
		}

		const R& GetRight () const
		{
			if (!IsRight ())
				throw std::runtime_error { "Tried accessing Right for a Left Either" };
			return boost::get<R> (This_);
		}

		boost::optional<L> MaybeLeft () const
		{
			if (!IsLeft ())
				return {};
			return GetLeft ();
		}

		boost::optional<R> MaybeRight () const
		{
			if (!IsRight ())
				return {};
			return GetRight ();
		}

		boost::variant<L, R> AsVariant () const
		{
			return This_;
		}

		template<typename RNew>
		static Either<L, RNew> FromMaybe (const boost::optional<RNew>& maybeRight, const L& left)
		{
			return maybeRight ?
					Either<L, RNew>::Right (*maybeRight) :
					Either<L, RNew>::Left (left);
		}

		static Either Left (const L& l)
		{
			return Either { l };
		}

		static Either Right (const R& r)
		{
			return Either { r };
		}

		friend bool operator== (const Either& e1, const Either& e2)
		{
			return e1.This_ == e2.This_;
		}

		friend bool operator!= (const Either& e1, const Either& e2)
		{
			return !(e1 == e2);
		}
	};

	template<template<typename> class Cont, typename L, typename R>
	std::pair<Cont<L>, Cont<R>> PartitionEithers (const Cont<Either<L, R>>& eithers)
	{
		std::pair<Cont<L>, Cont<R>> result;
		for (const auto& either : eithers)
			if (either.IsLeft ())
				result.first.push_back (either.GetLeft ());
			else
				result.second.push_back (either.GetRight ());

		return result;
	}

	template<typename L, typename R>
	struct InstanceFunctor<Either<L, R>>
	{
		template<typename F>
		using FmapResult_t = Either<L, ResultOf_t<F (R)>>;

		template<typename F>
		static FmapResult_t<F> Apply (const Either<L, R>& either, const F& f)
		{
			if (either.IsLeft ())
				return either;

			return FmapResult_t<F>::Right (f (either.GetRight ()));
		}
	};

	template<typename L, typename R>
	struct InstanceApplicative<Either<L, R>>
	{
		using Type_t = Either<L, R>;

		template<typename>
		struct GSLResult;

		template<typename V>
		struct GSLResult<Either<L, V>>
		{
			using Type_t = Either<L, ResultOf_t<R (V)>>;
		};

		static Type_t Pure (const R& v)
		{
			return Type_t::Right (v);
		}

		template<typename AV>
		static GSLResult_t<Type_t, AV> GSL (const Type_t& f, const AV& v)
		{
			using R_t = GSLResult_t<Type_t, AV>;

			if (f.IsLeft ())
				return R_t::Left (f.GetLeft ());

			if (v.IsLeft ())
				return R_t::Left (v.GetLeft ());

			return R_t::Right (f.GetRight () (v.GetRight ()));
		}
	};

	template<typename L, typename R>
	struct InstanceMonad<Either<L, R>>
	{
		template<typename F>
		using BindResult_t = ResultOf_t<F (R)>;

		template<typename F>
		static BindResult_t<F> Bind (const Either<L, R>& value, const F& f)
		{
			using R_t = BindResult_t<F>;

			if (value.IsLeft ())
				return R_t::Left (value.GetLeft ());

			return f (value.GetRight ());
		}
	};
}
}
