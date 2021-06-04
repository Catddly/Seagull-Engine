#pragma once

// Currently seagull have not any STL yet,
// use EASTL for STL library.
// Use a simple alias to be the warpper to EASTL for consistency
#include <EASTL/string_view.h>
namespace SG
{
	using string_view    = eastl::string_view;
	using wstring_view   = eastl::wstring_view;
	using u8string_view  = eastl::u8string_view;
	using u16string_view = eastl::u16string_view;
	using u32string_view = eastl::u32string_view;
}