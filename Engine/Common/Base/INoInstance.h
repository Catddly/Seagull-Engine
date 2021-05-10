#pragma once

namespace SG
{

	//! @Interface
	//! For some struct or class which do not
	//! want to have any instance
	struct INoInstance
	{
		INoInstance() = delete;
		~INoInstance() = delete;
		INoInstance(const INoInstance&) = delete;
		INoInstance operator=(const INoInstance&) = delete;
		INoInstance(const INoInstance&&) = delete;
		INoInstance operator=(const INoInstance&&) = delete;
	};

}