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
		class CFormatter
		{
		public:
			CFormatter() = default;
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