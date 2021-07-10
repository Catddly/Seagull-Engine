#pragma once

#include "Common/Config.h"
#include "Common/Base/BasicTypes.h"

namespace SG
{

	enum class EKeyCode
	{
		eBack = 0,
		eTab,
		eClear,
		eReturn,
		eLeftShift,
		eRightShift,
		eShift,
		eLeftControl,
		eRightControl,
		eControl,
		eLeftMenu,
		eRightMenu,
		eMenu,
		eBrowserBack,
		eBrowserForward,
		eBrowserRefresh,
		eBrowserStop,
		eBrowserSearch,
		eBrowserFavorites,
		eBrowserHome,
		ePause,
		eCapital,
		eEscape,
		eSpace,
		ePageUp,
		ePageDown,
		eEnd,
		eHome,
		eLeft,
		eUp,
		eRight,
		eDown,
		eSelect,
		ePrint,
		eExecute,
		eSnapShot,
		eInsert,
		eDelete,
		eHelp,
		eLeftWin,
		eRightWin,
		eApps,
		eSleep,
		eScrollLock,
		eVolumnMute,
		eVolumnUp,
		eVolumnDown,
		/// numpad
		eNumpad0,
		eNumpad1,
		eNumpad2,
		eNumpad3,
		eNumpad4,
		eNumpad5,
		eNumpad6,
		eNumpad7,
		eNumpad8,
		eNumpad9,
		eMultiply,
		eAdd,
		eSeparator,
		eSubtract,
		eDecimal,
		eDivide,
		eNumLock,
		/// Numbers
		e0,
		e1,
		e2,
		e3,
		e4,
		e5,
		e6,
		e7,
		e8,
		e9,
		/// letters
		eA,
		eB,
		eC,
		eD,
		eE,
		eF,
		eG,
		eH,
		eI,
		eJ,
		eK,
		eL,
		eM,
		eN,
		eO,
		eP,
		eQ,
		eR,
		eS,
		eT,
		eU,
		eV,
		eW,
		eX,
		eY,
		eZ,
		/// F-keys
		eF1,
		eF2,
		eF3,
		eF4,
		eF5,
		eF6,
		eF7,
		eF8,
		eF9,
		eF10,
		eF11,
		eF12,
		eF13,
		eF15,
		eF16,
		eF17,
		eF18,
		eF19,
		eF20,
		eF21,
		eF22,
		eF23,
		eF24,
		/// Mouse key
		eMouseLeft,
		eMouseRight,
		eMouseMiddle,
		eMouseScrollUp,
		eMouseScrollDown,
		eMouseScrollLeft,
		eMouseScrollRight,
		eMouse5,  //!< side-key 0
		eMouse6,  //!< side-key 1
		eMouse7,  //!< side-key 2

		KEYCODE_COUNT
	};

	enum class EKeyState
	{
		ePressed,
		eHold,
		eRelease,
	};

	struct IInput
	{
		SG_COMMON_API static bool IsKeyPressed(EKeyCode keycode);
	};

	//! Observer design pattern, can be register by any class which inherits this class.
	struct IInputListener
	{
		virtual ~IInputListener() = default;

		//! Call when there is any input.
		//! @param (keycode) which key is changing.
		//! @param (keyState) what is its state.
		//! @return if you want to propagate this event.
		virtual bool OnInputUpdate(EKeyCode keycode, EKeyState keyState) = 0;
	};

	struct IInputSystem
	{
		virtual ~IInputSystem() = default;

		virtual void RegisterListener(IInputListener* pListener) = 0;
		virtual void MuteListener(IInputListener* pListener) = 0;
		virtual void RemoveListener(IInputListener* pListener) = 0;

		virtual void OnUpdate() = 0;
	};

}