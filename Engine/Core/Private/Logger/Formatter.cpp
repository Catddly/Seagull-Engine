#include "StdAfx.h"

#include "Defs/Defs.h"
#include "Formatter.h"
#include "FormatType.h"

namespace SG
{
namespace fmt
{

	string Formatter::mCurrFormat = "";

	void Formatter::SetFormat(const char* format)
	{
		mCurrFormat = format;
	}

	string Formatter::GetFormattedString()
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
			case 'h': resStr += FormatString<STimeHourFormat>(); break;
			case 'm': resStr += FormatString<STimeMinuteFormat>(); break;
			case 's': resStr += FormatString<STimeSecondFormat>(); break;
			case 'y': resStr += FormatString<STimeYearFormat>(); break;
			case 'o': resStr += FormatString<STimeMonthFormat>(); break;
			case 'd': resStr += FormatString<STimeDayFormat>(); break;
			case 't': resStr += FormatString<SThreadFormat>(); break;
			default: SG_ASSERT(false && "Invalid format type found!"); break;
			}
			currPos = markPos + 2;
		}

		Size end = mCurrFormat.size();
		resStr += mCurrFormat.substr(currPos, end - currPos);

		return eastl::move(resStr);
	}

}
}