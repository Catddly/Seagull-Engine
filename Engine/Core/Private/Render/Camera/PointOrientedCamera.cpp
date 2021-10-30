#include "StdAfx.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mViewAtPoint(viewAt), mMoveSpeed(0.15f), mWheelScale(0.25f), BasicCamera(pos, {})
	{
		UpdateViewMatrix();
	}

	PointOrientedCamera::~PointOrientedCamera()
	{
	}

	void PointOrientedCamera::Update(float deltaTime)
	{

	}

	bool PointOrientedCamera::OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos)
	{
		if (Input::IsKeyPressed(KeyCode_MouseLeft))
		{
			if (deltaXPos != 0) Rotate(mViewMatrix, SG_ENGINE_UP_VEC(), -deltaXPos * mMoveSpeed);
			if (deltaYPos != 0) Rotate(mViewMatrix, -SG_ENGINE_RIGHT_VEC(), -deltaYPos * mMoveSpeed);
		}
		return true;
	}

	bool PointOrientedCamera::OnMouseWheelInputUpdate(int direction)
	{
		if (direction > 0) 
			{ TranslateZ(mViewMatrix, mWheelScale); }
		else 
			{ TranslateZ(mViewMatrix, -mWheelScale); }
		return true;
	}

	void PointOrientedCamera::UpdateViewMatrix()
	{
		mViewMatrix = BuildViewMatrixCenter(mPosition, mViewAtPoint, SG_ENGINE_UP_VEC());
	}

}