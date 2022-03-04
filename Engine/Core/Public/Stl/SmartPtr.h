#pragma once

#include "eastl/unique_ptr.h"
#include "eastl/shared_ptr.h"
#include "eastl/weak_ptr.h"

namespace SG
{

	template <typename T>
	using RefPtr = eastl::shared_ptr<T>;

	template <typename T, typename... Args>
	RefPtr<T> MakeRef(Args&&... args)
	{
		return eastl::make_shared<T>(eastl::forward<Args>(args)...);
	}

	template <typename T>
	using UniquePtr = eastl::unique_ptr<T>;

	template <typename T, typename... Args>
	UniquePtr<T> MakeUnique(Args&&... args)
	{
		return eastl::make_unique<T>(eastl::forward<Args>(args)...);
	}

	template <typename T>
	using WeakRefPtr = eastl::weak_ptr<T>;

}