#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

#include "Common/System/IModule.h"

#include <eigen/Dense>

namespace SG
{

	enum class EOsMessage
	{
		eNull = 0,
		eQuit,
	};

	//! Resolution in pixel.
	struct Resolution
	{
		UInt32 width;
		UInt32 height;

		bool operator==(const Resolution& rhs)
		{
			return width == rhs.width && height == rhs.height;
		}
		bool operator!=(const Resolution& rhs)
		{
			return !operator==(rhs);
		}
	};

	//! Rectangle to indicate an area on screen.
	struct Rect
	{
		Int32 left;
		Int32 top;
		Int32 right;
		Int32 bottom;

		bool operator==(const Rect& rhs) const noexcept
		{
			return left == rhs.left && top == rhs.top &&
				right == rhs.right && bottom == rhs.bottom;
		}
		bool operator!=(const Rect& rhs) const noexcept
		{
			return !(*this == rhs);
		}
	};

	//! Helper function to get the width of the SRect.
	static inline UInt32 GetRectWidth(const Rect& rect) { return rect.right - rect.left; }
	//! Helper function to get the height of the SRect.
	static inline UInt32 GetRectHeight(const Rect& rect) { return rect.bottom - rect.top; }

	// TODO: maybe put this on a message bus to transfer all the os messages.
	SG_COMMON_API EOsMessage PeekOSMessage();
	// TODO: return a EOsError as a error handle to propagate on message bus. 
	SG_COMMON_API void       PeekLastOSError();

	class Monitor;
	class Window;
	class Adapter;
	typedef Eigen::Matrix<Int32, 2, 1> Vector2i;

	struct IOperatingSystem : public IModule
	{
		virtual ~IOperatingSystem() = default;

		//! Get the monitor where the window lay on.
		virtual Monitor* GetMainMonitor() = 0;
		virtual UInt32   GetAdapterCount() = 0;
		virtual Adapter* GetPrimaryAdapter() = 0;
		//! Get the main window.
		virtual Window*  GetMainWindow() = 0;

		virtual Vector2i GetMousePos() const = 0;

		virtual bool     IsMainWindowOutOfScreen() const = 0;
	};

}