#include "StdAfx.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "System/System.h"
#include "Math/MathBasic.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera()
	{
		SSystem()->GetInputSystem()->RegisterListener(this);
	}

	PointOrientedCamera::PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt)
		:mPosition(pos), mViewAtPoint(viewAt)
	{
		CalcViewMatrix();
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
		Vector3f viewDirection = mPosition - mViewAtPoint;
		viewDirection.normalize();

		Vector3f worldUpVec = SG_ENGINE_UP_VEC();
		Vector3f rightVec = worldUpVec.cross(viewDirection);
		rightVec.normalize();

		Vector3f upVec = viewDirection.cross(rightVec);
		upVec.normalize();

		mViewMatrix.row(0) << rightVec.x(), rightVec.y(), rightVec.z(), 0.0f;
		mViewMatrix.row(1) << upVec.x(), upVec.y(), upVec.z(), 0.0f;
		mViewMatrix.row(2) << viewDirection.x(), viewDirection.y(), viewDirection.z(), 0.0f;
		mViewMatrix.row(3) << 0.0f, 0.0f, 0.0f, 1.0f;

		SG_LOG_MATH(ELogLevel::efLog_Level_Debug, mViewMatrix);
	}

}