#pragma once
#include "Core/Config.h"

#include "Core/STL/string.h"
#include "Core/STL/string_view.h"
#include <string>

namespace SG
{
	namespace fmt
	{
		//! %h, %m, %s are system hour, minute and second
		//! %y, %o, %d are system year, month and day
		//! %t means thread name
		class SG_CORE_API CFormatter
		{
		public:
			CFormatter() = default;
			explicit CFormatter(string_view format);
			~CFormatter() = default;

			//! Set the format of the formatter.
			void SetFormat(string_view format);
			//! Get the formatted string.
			string GetFormattedString();
		private:
			string mCurrFormat;
		};

	}
}