#include "StdAfx.h"
#include "Render/Camera/BasicCamera.h"

#include "Math/Transform.h"

namespace SG
{

	BasicCamera::BasicCamera(const Vector3f& position, const Vector3f& rotation)
		:mPosition(position), mRotation(rotation)
	{
		UpdateViewMatrix();
		Input::RegisterListener(this);
	}

	BasicCamera::~BasicCamera()
	{
		Input::RemoveListener(this);
	}

	void BasicCamera::SetPerspective(float fovyInDegrees, float aspect, float zNear, float zFar)
	{
		mProjectionMatrix = BuildPerspectiveMatrix(fovyInDegrees, aspect, zNear, zFar);
	}

	void BasicCamera::SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		mProjectionMatrix = BuildOrthographicMatrix(left, right, top, bottom, zNear, zFar);
	}

	void BasicCamera::UpdateViewMatrix()
	{
		const Vector3f DIRECTION = PitchYawToUnitVector(mRotation);
		mViewMatrix = BuildViewMatrixDirection(mPosition, DIRECTION, SG_ENGINE_UP_VEC());
		if (mRotation(2) - 0.0f >= SG_FLOAT_EPSILON) // if the rotation have the roll part
		{
			Transform rotationZ(AngleAxis(DegreesToRadians(mRotation(2)), Vector3f(0, 0, -1)));
			mViewMatrix = rotationZ.matrix() * mViewMatrix;
		}
	}

}