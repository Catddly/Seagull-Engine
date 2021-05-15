#ifndef TYPE_TRAITS_H
#define TYPE_TRAITS_H

#ifdef _MSC_VER
#	pragma once
#endif

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

	template <bool I0, bool I1, bool I2 = true, bool I3 = true, bool I4 = true>
	struct type_and { static const constexpr bool value = false; };

	template<>
	struct type_and<true, true, true, true, true> { static const constexpr bool value = true; };

	template <bool I0, bool I1, bool I2 = false, bool I3 = false, bool I4 = false>
	struct type_or { static const constexpr bool value = true; };

	template<>
	struct type_or<false, false, false, false, false> { static const constexpr bool value = false; };

	template <bool B>
	struct type_not { static const constexpr bool value = false; };

	template<>
	struct type_not<false> { static const constexpr bool value = true; };

namespace impl 
{
	template <typename T> struct is_const_value : public false_type {};
	template <typename T> struct is_const_value<const T*> : public true_type {};
	template <typename T> struct is_const_value<const volatile T*> : public true_type {};
}

	template <typename T> struct is_const : public impl::is_const_value<T*> {};
	template <typename T> struct is_const<T&> : public false_type {};
	template <class T> SG_CONSTEXPR bool is_const_v = is_const<T>::value;

	template <typename T> struct is_reference     : public false_type {};
	template <typename T> struct is_reference<T&> : public true_type {};
	template<typename T> SG_CONSTEXPR bool is_reference_v = is_reference<T>::value;

	//! If a type if a function, not include member function
	template <typename F>
	struct is_function : public false_type {};

	template <typename Return, typename... Args>
	struct is_function<Return(Args...)> : public true_type {};
	// use to check some functions like printf(format, ...);
	template <typename Return, typename... Args>
	struct is_function<Return(Args..., ...)> : public true_type {};
	template<typename T> SG_CONSTEXPR bool is_function_v = is_function<T>::value;

	template <typename T>
	struct is_class : public integral_constant<bool, __is_class(T)> {}; // use compile helper function to distinguish class
	template<typename T> SG_CONSTEXPR bool is_class_v = is_class<T>::value;

namespace impl
{
	template <typename T, bool = is_const_v<T> || is_reference_v<T> || is_function_v<T>>
	struct add_const_impl { typedef T type; };

	template <typename T>
	struct add_const_impl<T, false> { typedef const T type; };
}

	//! Add const to T
	template <typename T>
	struct add_const { typedef typename impl::add_const_impl<T>::type type; };

	//! remove reference of type T
	template<typename T> struct remove_ref     { typedef T type; };
	template<typename T> struct remove_ref<T&> { typedef T type; };
	template<typename T> using  remove_ref_t = typename remove_ref<T>::type;

	//! remove const of type T
	template<typename T> struct remove_const					   { typedef T type; };
	template<typename T> struct remove_const<const T>			   { typedef T type; };
	template<typename T> struct remove_const<const T[]>			   { typedef T type[]; };
	template<typename T, size_t N> struct remove_const<const T[N]> { typedef T type[N]; };
	template<typename T> using  remove_const_t = typename remove_const<T>::type;

	//! Add left reference to T
	//! void => void
	//! T    => T&
	//! T&   => T&
	//! T&&  => T&
	template<class T> struct add_lvalue_reference               { typedef T& type; };
	template<class T> struct add_lvalue_reference<T&>           { typedef T& type; };
	template<> struct add_lvalue_reference<void>                { typedef void type; };
	template<> struct add_lvalue_reference<const void>          { typedef const void type; };
	template<> struct add_lvalue_reference<volatile void>       { typedef volatile void type; };
	template<> struct add_lvalue_reference<const volatile void> { typedef const volatile void type; };
	template<typename T> using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

	//! Add right reference to T
	//! void => void
	//! T    => T&&
	//! T&   => T&
	//! T&&  => T&&
	template <typename T> struct add_rvalue_reference                      { typedef T&& type; };
	template <typename T> struct add_rvalue_reference<T&>                  { typedef T& type; };
	template <>           struct add_rvalue_reference<void>                { typedef void type; };
	template <>           struct add_rvalue_reference<const void>          { typedef const void type; };
	template <>           struct add_rvalue_reference<volatile void>       { typedef volatile void type; };
	template <>           struct add_rvalue_reference<const volatile void> { typedef const volatile void type; };
	template <typename T> using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

	template <typename T>
	typename add_rvalue_reference<T>::type declval() noexcept;

	template <typename T, typename U> struct is_same : public false_type { };
	template <typename T>             struct is_same<T, T> : public true_type { };
	template <class T, class U> SG_CONSTEXPR bool is_same_v = is_same<T, U>::value;

namespace impl
{
	template <typename T> struct is_mem_fun_pointer_value : public false_type {};

	template <typename R, typename T> struct is_mem_fun_pointer_value<R(T::*)()> : public true_type {};
	template <typename R, typename T> struct is_mem_fun_pointer_value<R(T::*)() const> : public true_type {};
	template <typename R, typename T> struct is_mem_fun_pointer_value<R(T::*)() volatile> : public true_type {};
	template <typename R, typename T> struct is_mem_fun_pointer_value<R(T::*)() const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0> struct is_mem_fun_pointer_value<R(T::*)(Arg0)> : public true_type {};
	template <typename R, typename T, typename Arg0> struct is_mem_fun_pointer_value<R(T::*)(Arg0) const> : public true_type {};
	template <typename R, typename T, typename Arg0> struct is_mem_fun_pointer_value<R(T::*)(Arg0) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0> struct is_mem_fun_pointer_value<R(T::*)(Arg0) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const volatile> : public true_type {};

	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) volatile> : public true_type {};
	template <typename R, typename T, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7> struct is_mem_fun_pointer_value<R(T::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const volatile> : public true_type {};
}
	template <typename T>
	struct is_member_function_pointer : public integral_constant<bool, impl::is_mem_fun_pointer_value<T>::value> {};
	template<typename T> SG_CONSTEXPR bool is_member_function_pointer_v = is_member_function_pointer<T>::value;

	template <typename T>
	struct is_member_pointer : public integral_constant<bool, is_member_function_pointer<T>::value> {};
	template <typename T, typename U>
	struct is_member_pointer<U T::*> : public true_type {};
	template<typename T> SG_CONSTEXPR bool is_member_pointer_v = is_member_pointer<T>::value;

namespace impl
{
	template <typename T> struct is_pointer_impl : public false_type {};

	template <typename T> struct is_pointer_impl<T*> : public true_type {};
	template <typename T> struct is_pointer_impl<T* const> : public true_type {};
	template <typename T> struct is_pointer_impl<T* volatile> : public true_type {};
	template <typename T> struct is_pointer_impl<T* const volatile> : public true_type {};

	template <typename T>
	struct is_pointer_value : public type_and<impl::is_pointer_impl<T>::value, type_not<is_member_pointer<T>::value>::value> {};
}
	
	//! If T is a pointer, but not a class member pointer or member function pointer
	template <typename T>
	struct is_pointer : public integral_constant<bool, impl::is_pointer_value<T>::value> {};
	template<typename T> SG_CONSTEXPR bool is_pointer_v = is_pointer<T>::value;

namespace impl
{
	template<typename T, typename U>
	struct is_assignable_impl
	{
		template<typename, typename>
		static no_type is(...);

		template<typename T1, typename U1>
		static decltype(declval<T1>() = declval<U1>(), yes_type()) is(int);

		static const bool value = (sizeof(is<T, U>(0)) == sizeof(yes_type));
	};
}

	template<typename T, typename U>
	struct is_assignable : public integral_constant<bool, impl::is_assignable_impl<T, U>::value> {};

	template <bool B, typename T, typename U>
	struct is_trivially_assignable_impl;
	template <typename T, typename U>
	struct is_trivially_assignable_impl<true, T, U> : integral_constant<bool, __is_trivially_assignable(T, U)> {}; // compiler helper function
	template <typename T, typename U>
	struct is_trivially_assignable_impl<false, T, U> : false_type {};

	template <typename T, typename U>
	struct is_trivially_assignable : integral_constant<bool, is_trivially_assignable_impl<is_assignable<T, U>::value, T, U >::value> {};

	//! Check if a ADT is trivially_copy_assignable
	//! T shall be a complete type, void, or an array of unknown bound.
	template<class T>
	struct is_trivially_copy_assignable : public is_trivially_assignable<typename add_lvalue_reference<T>::type,
		typename add_lvalue_reference<typename add_const<T>::type>::type> {};

	template <typename T>
	struct is_trivially_copyable { static const bool value = __is_trivially_copyable(T); };
	template <class T> SG_CONSTEXPR bool is_trivially_copyable_v = is_trivially_copyable<T>::value;

	template <bool B, typename T = void> // if unable, then the type will be void
	struct enable_if {};
	template <typename T>
	struct enable_if<true, T> { typedef T type; };
	template <bool B, typename T = void>
	using  enable_if_t = typename enable_if<B, T>::type;

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