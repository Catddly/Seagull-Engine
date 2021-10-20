#include "StdAfx.h"
#include "Render/Camera/PointOrientedCamera.h"

#include "Math/MathBasic.h"
#include "Math/Transform.h"

namespace SG
{

	PointOrientedCamera::PointOrientedCamera()
		:mViewMatrix(Matrix4f::Identity()), mPosition({ 0.0f, 0.0f, -3.0f }), mViewAtPoint(Vector3f::Zero())
	{
		CalcViewMatrix();
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

	void PointOrientedCamera::SetPerspective(float fovyInDegrees, float aspect, float zNear, float zFar)
	{
		mProjectionMatrix = BuildPerspectiveMatrix(DegreesToRadians(fovyInDegrees), aspect, zNear, zFar);
	}

	void PointOrientedCamera::SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		mProjectionMatrix = BuildOrthographicMatrix(left, right, top, bottom, zNear, zFar);
	}

	bool PointOrientedCamera::OnInputUpdate(EKeyCode keycode, EKeyState keyState)
	{
		return true;
	}

	void PointOrientedCamera::CalcViewMatrix()
	{
		mViewMatrix = BuildViewMatrix(mPosition, mViewAtPoint, SG_ENGINE_UP_VEC());
	}

}