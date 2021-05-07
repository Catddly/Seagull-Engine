#pragma once

namespace SG
{
	//! @Interface
	//! Design pattern singleton
	template <typename T>
	struct ISingleton
	{
		ISingleton() = default;
		virtual ~ISingleton() = default;
		ISingleton(const ISingleton&) = delete;
		ISingleton operator=(const ISingleton&) = delete;

		static T* GetInstance() { static T instance; return &instance; }
	};

}