#include "StdAfx.h"
#include "Formatter.h"

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"
#include "FormatType.h"

namespace SG
{
	namespace fmt
	{

		void formatter::set_format(std::string_view format)
		{
			auto instance = GetInstance();
			instance->mCurrFormat = format;
		}

		std::string formatter::format()
		{
			auto instance = GetInstance();
			auto& currFormat = instance->mCurrFormat;

			if (currFormat == "")
				return "";

			Size currPos = 0;
			Size markPos = 0;
			std::string resStr = "";
			while (true)
			{
				markPos = currFormat.find_first_of('%', currPos);
				if (markPos == std::string::npos)
					break;

				char mark = currFormat[markPos + 1];
				resStr += currFormat.substr(currPos, markPos - currPos);

				switch (mark)
				{
					case 'h': resStr += format_type<STimeHourFormat>(); break;
					case 'm': resStr += format_type<STimeMinuteFormat>(); break;
					case 's': resStr += format_type<STimeSecondFormat>(); break;
					case 'y': resStr += format_type<STimeYearFormat>(); break;
					case 'o': resStr += format_type<STimeMonthFormat>(); break;
					case 'd': resStr += format_type<STimeDayFormat>(); break;
					case 't': resStr += format_type<SThreadFormat>(); break;
					default: SG_ASSERT(false); break;
				}
				currPos = markPos + 2;
			}

			Size end = currFormat.size();
			resStr += currFormat.substr(currPos, end - currPos);

			return std::move(resStr);
		}

	}
}