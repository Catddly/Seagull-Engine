#include "StdAfx.h"

#include "Defs/Defs.h"
#include "Formatter.h"
#include "FormatType.h"

namespace SG
{
	namespace fmt
	{

		void CFormatter::SetFormat(string_view format)
		{
			mCurrFormat = format;
		}

		string CFormatter::GetFormattedString()
		{
			if (mCurrFormat == "")
				return "";

			Size currPos = 0;
			Size markPos = 0;
			string resStr;
			while (true)
			{
				markPos = mCurrFormat.find_first_of('%', currPos);
				if (markPos == string::npos)
					break;

				char mark = mCurrFormat[markPos + 1];
				resStr += mCurrFormat.substr(currPos, markPos - currPos);

				switch (mark)
				{
				case 'h': resStr += format_type<STimeHourFormat>(); break;
				case 'm': resStr += format_type<STimeMinuteFormat>(); break;
				case 's': resStr += format_type<STimeSecondFormat>(); break;
				case 'y': resStr += format_type<STimeYearFormat>(); break;
				case 'o': resStr += format_type<STimeMonthFormat>(); break;
				case 'd': resStr += format_type<STimeDayFormat>(); break;
				case 't': resStr += format_type<SThreadFormat>(); break;
				default: SG_ASSERT(false && "Invalid format type found!"); break;
				}
				currPos = markPos + 2;
			}

			Size end = mCurrFormat.size();
			resStr += mCurrFormat.substr(currPos, end - currPos);

			return move(resStr);
		}

	}
}