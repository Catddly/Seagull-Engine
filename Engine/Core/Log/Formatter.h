#pragma once
#include "Core/Config.h"

#include "Common/Base/ISingleton.h"

#include "Core/STL/string.h"
//#include "Core/STL/string_view.h"
#include <string>

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %y, %o, %d are system year, month and day
		//! %t means thread name
		class formatter : ISingleton<formatter>
		{
		public:
			SG_CORE_API static void set_format(string_view format);
			SG_CORE_API static std::string format();
		private:
			std::string mCurrFormat;
		};

	}
}