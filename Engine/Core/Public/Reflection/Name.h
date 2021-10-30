#pragma once

#include <eastl/string.h>
#include <eastl/string_view.h>

namespace SG
{
namespace Refl
{
	
	template <typename Class>
	constexpr const char* CT_TypeName()
	{
#ifdef SG_COMPILER_MSVC
		eastl::string_view fullName(__FUNCSIG__);
		size_t leftAngleBracket  = fullName.find_last_of('<');
		size_t rightAngleBracket = fullName.find_last_of('>');
		eastl::string_view name = fullName.substr(leftAngleBracket + 1, rightAngleBracket - leftAngleBracket - 1);
		if (name.find("struct") != std::string_view::npos)
			return eastl::string(name.substr(7, name.size() - 7)).c_str();
		else if (name.find("class") != std::string_view::npos)
			return eastl::string(name.substr(6, name.size() - 6)).c_str();
		else
			return eastl::string(name).c_str();
#endif
	}

}
}