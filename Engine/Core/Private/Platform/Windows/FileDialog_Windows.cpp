#include "StdAfx.h"
#ifdef SG_PLATFORM_WINDOWS

#include "Platform/FileDialog.h"

namespace SG
{

namespace FileDialog
{

	string OpenFileDialog(Window* pWindow, const char* filter)
	{
		OPENFILENAMEA ofn = {};
		CHAR szFile[260] = { 0 };

		// initialize OPENFILENAME
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = reinterpret_cast<HWND>(pWindow->GetNativeHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return "";
	}

	string SaveFileDialog(Window* pWindow, const char* filter)
	{
		OPENFILENAMEA ofn = {};
		CHAR szFile[260] = { 0 };

		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = reinterpret_cast<HWND>(pWindow->GetNativeHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return "";
	}

}

}

#endif // SG_PLATFORM_WINDOWS