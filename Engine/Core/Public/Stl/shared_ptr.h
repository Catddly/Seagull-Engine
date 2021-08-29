#pragma once

#include <EASTL/shared_ptr.h>

namespace SG
{

	template <class T>
	using SharedPtr = eastl::shared_ptr<T>;

	template <class T>
	using UniquePtr = eastl::unique_ptr<T>;

	template <class T>
	using WeakPtr   = eastl::weak_ptr<T>;

}