#pragma once
#include "Core/Config.h"

#include "Stl/string.h"
#include "Stl/string_view.h"

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %y, %o, %d are system year, month and day
		//! %t means thread name
		class Formatter
		{
		public:
			//! Set the format of the formatter.
			SG_CORE_API static void SetFormat(const char* format);
			//! Get the formatted string.
			SG_CORE_API static string GetFormattedString();
		private:
			static string mCurrFormat;
		};

	}
}