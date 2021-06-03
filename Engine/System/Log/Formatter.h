#pragma once
#include "Core/Config.h"

#include <EASTL/string.h>
#include <EASTL/string_view.h>

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %y, %o, %d are system year, month and day
		//! %t means thread name
		class CFormatter
		{
		public:
			CFormatter() = default;
			~CFormatter() = default;

			//! Set the format of the formatter.
			void SetFormat(eastl::string_view format);
			//! Get the formatted string.
			eastl::string GetFormattedString();
		private:
			eastl::string mCurrFormat;
		};

	}
}