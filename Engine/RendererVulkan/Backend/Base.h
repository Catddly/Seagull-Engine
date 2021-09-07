#pragma once

template <typename T>
class VkSmartPtr
{
public:
	VkSmartPtr(T* ptr) {}
	~VkSmartPtr() {}

	T operator*()
	{
		if (ptr != VK_NULL_HANDLE)
			return T;
		else
			return VK_NULL_HANDLE;
	}

	T operator*() const
	{
		if (ptr != VK_NULL_HANDLE)
			return T;
		else
			return VK_NULL_HANDLE;
	}

	T& operator->()
	{
		return **T;
	}


	T& operator->() const
	{
		return **T;
	}
private:
	T* ptr = nullptr;
};