#pragma once

#include "eastl/utility.h"

namespace SG
{

#define SG_FWD(...) eastl::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

#define SG_BIND_FUNC(f)        [](auto&&... args) -> decltype(auto) { return f(eastl::forward<decltype(args)>(args)...); }
#define SG_BIND_MEMBER_FUNC(f) [this](auto&&... args) -> decltype(auto) { return this->f(eastl::forward<decltype(args)>(args)...); }

}