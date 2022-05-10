#pragma once

#include "Platform/OS.h"

#include "Stl/string.h"

namespace SG
{

namespace FileDialog
{

	string OpenFileDialog(Window* pWindow, const char* filter);
	string SaveFileDialog(Window* pWindow, const char* filter);

}

}