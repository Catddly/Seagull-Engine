#include "StdAfx.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mViewAtPoint(viewAt), BasicCamera(pos, {})
	{
	}

	PointOrientedCamera::~PointOrientedCamera()
	{
	}

	bool PointOrientedCamera::OnInputUpdate(EKeyCode keycode, EKeyState keyState, int xPos, int yPos)
	{
		return true;
	}

	void PointOrientedCamera::Update(float deltaTime)
	{

	}

}