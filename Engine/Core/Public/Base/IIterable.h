#pragma once

#include "Defs/Defs.h"

namespace SG
{

	template <typename T>
	interface IIterator
	{
	public:
		virtual ~IIterator() = default;

		virtual bool HaveNext() const = 0;
		virtual T    Next() const = 0;
	};

	template <typename T>
	interface IContainer
	{
	public:
		virtual ~IContainer() = default;

		virtual IIterator<T> GetIterator() const = 0;
	};

}