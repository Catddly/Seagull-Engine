#include "StdAfx.h"
#include "Scene/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mViewAtPoint(viewAt), mMoveSpeed(0.15f), mWheelScale(0.25f), BasicCamera(pos, Vector3f(0.0f))
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
			if (deltaXPos != 0) { mPosition.x += deltaXPos * mMoveSpeed * 0.05f; UpdateViewMatrix(); mbIsViewDirty = true; }
			if (deltaYPos != 0) { mPosition.y -= deltaYPos * mMoveSpeed * 0.05f; UpdateViewMatrix(); mbIsViewDirty = true; }
		}
		return true;
	}

	bool PointOrientedCamera::OnMouseWheelInputUpdate(int direction)
	{
		if (direction > 0)
		{
			//glm::translate(mViewMatrix, { 0.0f, 0.0f, mWheelScale });
			mPosition.z += mWheelScale * 0.7f;
			UpdateViewMatrix();
			mbIsViewDirty = true;
		}
		else
		{
			mPosition.z -= mWheelScale * 0.7f;
			UpdateViewMatrix();
			//glm::translate(mViewMatrix, { 0.0f, 0.0f, -mWheelScale });
			mbIsViewDirty = true;
		}
		return true;
	}

	void PointOrientedCamera::UpdateViewMatrix()
	{
		mViewMatrix = BuildViewMatrixCenter(mPosition, mViewAtPoint, SG_ENGINE_UP_VEC());
	}

}