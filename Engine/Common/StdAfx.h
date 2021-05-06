#pragma once

#include <cmath>

#if defined(SG_DLL)
#define SG_CORE_API __declspec(dllexport)
#else
#define SG_CORE_API __declspec(dllimport)
#endif