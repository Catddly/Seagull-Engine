#pragma once

// Currently seagull have not any STL yet,
// use EASTL for STL library.
// Use a simple alias to be the warpper to EASTL for consistency
#include <EASTL/vector.h>
namespace SG
{
	template<class T> using vector = eastl::vector<T>;
}