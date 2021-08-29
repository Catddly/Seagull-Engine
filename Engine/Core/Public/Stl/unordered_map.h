#pragma once

// Currently seagull have not any STL yet,
// use EASTL for STL library.
// Use a simple alias to be the warpper to EASTL for consistency
#include <EASTL/unordered_map.h>
namespace SG
{
	template<class Key, class Val> using unordered_map = eastl::unordered_map<Key, Val>;
}