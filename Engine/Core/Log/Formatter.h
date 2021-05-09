#pragma once
#include "../Config.h"

#include <string_view>
#include "Common/Base/ISingleton.h"

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %t means thread name
		class SG_LOG_API formatter : ISingleton<formatter>
		{
		public:
			static void set_format(std::string_view format);
			static std::string format();
		private:
			std::string mCurrFormat;
		};

	}
}