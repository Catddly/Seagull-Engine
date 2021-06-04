#pragma once

// Currently seagull have not any STL yet,
// use EASTL for STL library.
// Use a simple alias to be the warpper to EASTL for consistency
#include <EASTL/string.h>
namespace SG
{
	using string    =  eastl::string;
	using wstring   =  eastl::wstring;
	using u8string  =  eastl::string8;
	using u16string =  eastl::string16;
	using u32string =  eastl::string32;
}