#include "StdAfx.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "System/System.h"
#include "Platform/Window.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera()
		:mViewMatrix(Matrix4f::Identity()), mPosition({ 0.0f, 0.0f, -3.0f }), mViewAtPoint(Vector3f::Zero())
	{
		CalcViewMatrix();
		CalcPerspectiveMatrix();
		SSystem()->GetInputSystem()->RegisterListener(this);
	}

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mPosition(pos), mViewAtPoint(viewAt)
	{
		CalcViewMatrix();
		CalcPerspectiveMatrix();
		SSystem()->GetInputSystem()->RegisterListener(this);
	}

	PointOrientedCamera::~PointOrientedCamera()
	{
		SSystem()->GetInputSystem()->RemoveListener(this);
	}

	bool PointOrientedCamera::OnInputUpdate(EKeyCode keycode, EKeyState keyState)
	{
		return true;
	}

	void PointOrientedCamera::CalcViewMatrix()
	{
//#ifdef SG_GRAPHICS_API_VULKAN
//		mPosition(1) *= -1.0f; // flip y coordinate
//#endif
		mViewMatrix = BuildViewMatrix(mPosition, mViewAtPoint, SG_ENGINE_UP_VEC());
	}

	void PointOrientedCamera::CalcPerspectiveMatrix()
	{
		auto* pWindow = SSystem()->GetOS()->GetMainWindow();
		const UInt32 WIDTH  = pWindow->GetWidth();
		const UInt32 HEIGHT = pWindow->GetHeight();

		mPerspectiveMatrix = BuildPerspectiveMatrix(DegreesToRadians(45.0f), (float)WIDTH / (float)HEIGHT, 1.0f, 1000.0f);
//#ifdef SG_GRAPHICS_API_VULKAN
//		mPerspectiveMatrix(1, 1) *= -1.0f; // flip y coordinate
//#endif
	}

}