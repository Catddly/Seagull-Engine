#pragma once

#include "Base/BasicTypes.h"
#include "Defs/Defs.h"

namespace SG
{

	class ResourcePool;
	class RID
	{
	public:
		~RID() = default;
		SG_CLASS_NO_COPY_ASSIGNABLE(RID);

		bool IsValid() const;
		bool IsInitialized() const;

		bool operator<(const RID& rhs)  const { return (UInt32)(mID & ID_MASK) < (UInt32)(mID & ID_MASK); }
		bool operator>(const RID& rhs)  const { return (UInt32)(mID & ID_MASK) > (UInt32)(mID & ID_MASK); }
		bool operator==(const RID& rhs) const { return (UInt32)(mID & ID_MASK) == (UInt32)(mID & ID_MASK); }
		bool operator<=(const RID& rhs) const { return (UInt32)(mID & ID_MASK) <= (UInt32)(mID & ID_MASK); }
		bool operator>=(const RID& rhs) const { return (UInt32)(mID & ID_MASK) >= (UInt32)(mID & ID_MASK); }
	private:
		static const UInt32 ID_MASK          = 0x00FFffFF;
		static const UInt32 VALID_MASK       = 0x80000000;
		static const UInt32 INITIALIZED_MASK = 0x40000000;

		friend class ResourcePool;
		RID() = default;
	private:
		UInt32 mID = 0;
	};

}