#pragma once

#include "TipECS/Config.h"

#include <tuple>
#include <array>
#include <type_traits>

namespace TMP
{

	// Same implementation of boost::strong_typedef
#define STRONG_TYPE_DEFS(TYPE, NAME) \
	struct NAME { \
	TYPE data; \
	explicit NAME(const TYPE& d) : data(d) {} \
	NAME() : data() {} \
	NAME(const NAME& d) : data(d.data) {} \
	NAME& operator=(const NAME& rhs) { data = rhs.data; return *this; } \
	NAME& operator=(const TYPE& rhs) { data = rhs; return *this; } \
	operator const TYPE& () const { return data; } \
	operator TYPE& () { return data; } \
	bool operator==(const NAME& rhs) const { return data == rhs.data; } \
	bool operator<(const NAME& rhs) const { return data < rhs.data; } \
	friend bool operator!=(const NAME& x, const NAME& y) { return !static_cast<bool>(x == y); } \
    friend bool operator> (const NAME& x, const NAME& y) { return y < x; } \
	friend bool operator<=(const NAME& x, const NAME& y) { return !static_cast<bool>(y < x); } \
	friend bool operator>=(const NAME& x, const NAME& y) { return !static_cast<bool>(x < y); } \
    friend bool operator==(const TYPE& y, const NAME& x) { return x == y; } \
	friend bool operator!=(const TYPE& y, const NAME& x) { return !static_cast<bool>(x == y); } \
	friend bool operator!=(const NAME& y, const TYPE& x) { return !static_cast<bool>(y == x); } \
    friend bool operator<=(const NAME& x, const TYPE& y) { return !static_cast<bool>(x > y); } \
	friend bool operator>=(const NAME& x, const TYPE& y) { return !static_cast<bool>(x < y); } \
	friend bool operator> (const TYPE& x, const NAME& y) { return y < x; } \
	friend bool operator< (const TYPE& x, const NAME& y) { return y > x; } \
	friend bool operator<=(const TYPE& x, const NAME& y) { return !static_cast<bool>(y < x); } \
	friend bool operator>=(const TYPE& x, const NAME& y) { return !static_cast<bool>(y > x); } \
	}

#define FWD(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

	/// begin namespace Impl
	namespace Impl
	{

		// Thanks to https://stackoverflow.com/questions/25958259/how-do-i-find-out-if-a-tuple-contains-a-type
		template <typename T, typename TTuple>
		struct HasType;

		template <typename T, typename... Ts>
		struct HasType<T, std::tuple<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {}; // expand to if see each elements is T.

		template <size_t I, typename T, typename TTuple>
		constexpr size_t IndexOf()
		{
			static_assert(HasType<T, TTuple>::value, "T is not a member of TypeList!");

			typedef typename std::tuple_element<I, TTuple>::type element_t; // try get the type of the I tuple element
			if constexpr (std::is_same_v<T, element_t>)
			{
				return I;
			}
			else
			{
				return IndexOf<I + 1, T, TTuple>();
			}
		}
	}
	/// end   namespace Impl

	//! Simple wrapper of tuple.
	template <typename... Ts>
	class TypeList
	{
	private:
		template <typename... Ts>
		using ThisType = std::tuple<Ts...>;
	public:
		static constexpr size_t INVALID_INDEX = size_t(-1);

		template <typename T>
		static constexpr auto Contain() -> bool
		{
			return Impl::HasType<T, ThisType<Ts...>>::value;
		}

		template <typename T>
		static constexpr auto At() -> size_t
		{
			return Impl::IndexOf<0, T, ThisType<Ts...>>();
		}

		static constexpr auto Size() -> size_t
		{
			return std::tuple_size_v<ThisType<Ts...>>;
		}
	};

	//! Convert a TypeList to tuple.
	template <typename TTypeList>
	struct ToTuple;

	template <typename... Ts>
	struct ToTuple<TypeList<Ts...>>
	{
		using type = std::tuple<Ts...>;
	};

	//! Convert a tuple to TypeList.
	template <typename TTuple>
	struct ToTypeList;

	template <typename... Ts>
	struct ToTypeList<std::tuple<Ts...>>
	{
		using type = typename TypeList<Ts...>;
	};

	template <template<typename...> typename Expand, typename TTypeList>
	struct Unpack;

	template <template<typename...> typename Expand, typename... Ts>
	struct Unpack<Expand, TypeList<Ts...>>
	{
		using type = Expand<Ts...>;
	};

	/// begin namespace Impl
	namespace Impl
	{
		template <typename TTuple, typename TFunc>
		constexpr void Apply_Impl(TFunc&& func)
		{
			TTuple tuple;
			std::apply(func, tuple);
		}
	}
	/// end   namespace Impl

	//! For all the types in TypeList, call the lambda function
	template <typename TTypeList, typename TFunc>
	constexpr void Apply(TFunc&& func)
	{
		using tuple_t = typename ToTuple<TTypeList>::type;
		Impl::Apply_Impl<tuple_t>(func);
	}

	/// begin namespace Impl
	namespace Impl
	{
		//! Filter an arbitrary tuple using type Pred (predicator must have value member)
		template <template<typename> typename Pred, typename TTuple>
		class Filter_Impl;

		template <template<typename> typename Pred, typename... Ts>
		class Filter_Impl<Pred, std::tuple<Ts...>>
		{
		private:
			// do sink or lift
			template <class T>
			using Filter_Select = std::conditional_t<Pred<T>::value,
				std::tuple<T>, std::tuple<>>;
		public:
			using type = decltype(std::tuple_cat(std::declval<Filter_Select<Ts>>()...));
		};
	}
	/// end   namespace Impl

	//! Filter a TypeList using Pred (predicator must have an bool value).
	template <template<typename> typename Pred, typename TTypeList>
	class Filter;

	template <template<typename> typename Pred, typename... Ts>
	class Filter<Pred, TypeList<Ts...>>
	{
	private:
		using filter_t = typename Impl::Filter_Impl<Pred, std::tuple<Ts...>>::type;
	public:
		using type = typename ToTypeList<filter_t>::type;
	};

	/// begin namespace Impl
	namespace Impl
	{
		template <typename Array, size_t... I>
		constexpr auto MakeTupleN_Impl(const Array& array, std::index_sequence<I...>)
		{
			return std::make_tuple<>(array[I]...);
		}
	}
	/// end   namespace Impl

	template <size_t N, typename T, typename Indices = std::make_index_sequence<N>>
	constexpr auto MakeTupleN()
	{
		return Impl::MakeTupleN_Impl(std::array<T, N>{}, Indices{});
	}

	namespace Impl
	{
		template <typename TTuple, typename TFunc, size_t... I>
		constexpr void ForTuple_ImplCall(TTuple& tuple, TFunc&& func, std::index_sequence<I...>)
		{
			// dirty way to expand the template parameters inside a function.
			using _dummy = int[];
			(void)_dummy { 0, (func(std::get<I>(tuple)), 0)... };
		}

		template <size_t N, typename TTuple, typename TFunc, typename Indices = std::make_index_sequence<N>>
		constexpr void ForTuple_Impl(TTuple& tuple, TFunc&& func)
		{
			ForTuple_ImplCall(tuple, std::forward<TFunc>(func), Indices{});
		}
	}

	template <typename TTuple, typename TFunc>
	constexpr void ForTuple(TTuple& tuple, TFunc&& func)
	{
		Impl::ForTuple_Impl<std::tuple_size<TTuple>::value>(tuple, std::forward<TFunc>(func));
	}

}