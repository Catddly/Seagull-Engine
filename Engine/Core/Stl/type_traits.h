#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

#pragma once

#include "Common/Core/Defs.h"

namespace SG
{

	template<class T, T val>
	struct integral_constant
	{
		typedef T value_type;
		typedef integral_constant<T, val> this_type;

		static SG_CONSTEXPR T value = val;

		constexpr value_type operator()() const noexcept { return value; }
		constexpr operator   value_type() const noexcept { return value; }
	};

	typedef integral_constant<bool, true>  true_type;
	typedef integral_constant<bool, false> false_type;

	typedef char yes_type;
	struct       no_type { char pad[8]; };

	//! remove reference of type T
	template<typename T> struct remove_ref     { typedef T type; };
	template<typename T> struct remove_ref<T&> { typedef T type; };
	template<typename T> using  remove_ref_t = typename remove_ref<T>::type;

	//! remove const of type T
	template<typename T> struct remove_const					   { typedef T type; };
	template<typename T> struct remove_const<const T>			   { typedef T type; };
	template<typename T> struct remove_const<const T[]>			   { typedef T type[]; };
	template<typename T, size_t N> struct remove_const<const T[N]> { typedef T type[N]; };

	template<class T>
	SG_CONSTEXPR typename remove_ref<T>::type&& move(T&& val) noexcept
	{
		return ((typename remove_ref<T>::type&&)val);
	}

	template<class T>
	SG_CONSTEXPR T&& forward(typename remove_ref<T>::type& args) noexcept
	{
		return static_cast<T&&>(args);
	}

	template<class T>
	SG_CONSTEXPR T&& forward(typename remove_ref<T>::type&& args) noexcept
	{
		return static_cast<T&&>(args);
	}

}

#endif // TYPE_TRAITS_H