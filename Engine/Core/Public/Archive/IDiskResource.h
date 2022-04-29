#pragma once

#include "Defs/Defs.h"

#include "Stl/string.h"

namespace SG
{

	interface IDiskResource
	{
		virtual ~IDiskResource() = default;

		virtual void LoadDataFromDisk() noexcept = 0;

		virtual string GetFileName() const noexcept = 0;
		virtual string GetFilePath() const noexcept = 0;
	};

}