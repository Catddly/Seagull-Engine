#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

namespace SG
{

	typedef enum class EOsMessage
	{
		eNull = 0,
		eQuit,
	} EOsMessage;

	//! Resolution in pixel.
	typedef struct SResolution
	{
		UInt32 width;
		UInt32 height;

		bool operator==(const SResolution& rhs)
		{
			return width == rhs.width && height == rhs.height;
		}
		bool operator!=(const SResolution& rhs)
		{
			return !operator==(rhs);
		}
	} SResolution;

	//! Rectangle to indicate an area on screen.
	typedef struct SRect
	{
		Int32 left;
		Int32 top;
		Int32 right;
		Int32 bottom;

		bool operator==(const SRect& rhs) const noexcept
		{
			return left == rhs.left && top == rhs.top &&
				right == rhs.right && bottom == rhs.bottom;
		}
		bool operator!=(const SRect& rhs) const noexcept
		{
			return !(*this == rhs);
		}
	} SRect;

	//! Helper function to get the width of the SRect.
	static inline UInt32 GetRectWidth(const SRect& rect) { return rect.right - rect.left; }
	//! Helper function to get the height of the SRect.
	static inline UInt32 GetRectHeight(const SRect& rect) { return rect.bottom - rect.top; }

	// TODO: maybe put this on a message bus to transfer all the os messages.
	SG_COMMON_API EOsMessage PeekOSMessage();

	struct IOperatingSystem
	{
		virtual ~IOperatingSystem() = default;

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;
	};

}