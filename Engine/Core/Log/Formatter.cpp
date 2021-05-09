#include "StdAfx.h"
#include "Formatter.h"

#include "Common/Core/Defs.h"
#include "Common/Base/BasicTypes.h"

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
					case 'h': resStr += "!hour!"; break;
					case 'm': resStr += "!minute!"; break;
					case 's': resStr += "!second!"; break;
					case 't': resStr += "!thread!"; break;
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