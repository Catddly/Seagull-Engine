#pragma once
#include "Core/Config.h"

#include <string_view>
#include "Common/Base/ISingleton.h"

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %y, %o, %d are system year, month and day
		//! %t means thread name
		class SG_CORE_API formatter : ISingleton<formatter>
		{
		public:
			static void set_format(std::string_view format);
			static std::string format();
		private:
			std::string mCurrFormat;
		};

	}
}