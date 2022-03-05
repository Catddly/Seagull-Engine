#include "StdAfx.h"
#include "Scene/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mViewAtPoint(viewAt), mMoveSpeed(0.15f), mWheelScale(0.25f), BasicCamera(pos, Vector3f::Zero())
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
			if (deltaXPos != 0) { Rotate(mViewMatrix, SG_ENGINE_UP_VEC(), -deltaXPos * mMoveSpeed); mbIsViewDirty = true; }
			if (deltaYPos != 0) { Rotate(mViewMatrix, SG_ENGINE_RIGHT_VEC(), deltaYPos * mMoveSpeed); mbIsViewDirty = true; }
		}
		return true;
	}

	bool PointOrientedCamera::OnMouseWheelInputUpdate(int direction)
	{
		if (direction > 0) 
		{
			TranslateZ(mViewMatrix, mWheelScale); 
			mbIsViewDirty = true;
		}
		else 
		{
			TranslateZ(mViewMatrix, -mWheelScale);
			mbIsViewDirty = true;
		}
		return true;
	}

	void PointOrientedCamera::UpdateViewMatrix()
	{
		mViewMatrix = BuildViewMatrixCenter(mPosition, mViewAtPoint, SG_ENGINE_UP_VEC());
	}

}