#include "Stdafx.h"
#include "Archive/RID.h"

namespace SG
{

	bool RID::IsValid() const
	{
		return (mID & VALID_MASK);
	}

	bool RID::IsInitialized() const
	{
		return (mID & INITIALIZED_MASK);
	}

}