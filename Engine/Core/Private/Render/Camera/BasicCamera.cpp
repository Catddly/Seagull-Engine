#include "StdAfx.h"
#include "Render/Camera/BasicCamera.h"

#include "Math/Transform.h"

namespace SG
{

	BasicCamera::BasicCamera(const Vector3f& position, const Vector3f& rotation)
		:mPosition(position), mRotation(rotation)
	{
		UpdateViewMatrix();
		SSystem()->GetInputSystem()->RegisterListener(this);
	}

	BasicCamera::~BasicCamera()
	{
		SSystem()->GetInputSystem()->RemoveListener(this);
	}

	void BasicCamera::SetPerspective(float fovyInDegrees, float aspect, float zNear, float zFar)
	{
		mProjectionMatrix = BuildPerspectiveMatrix(fovyInDegrees, aspect, zNear, zFar);
	}

	void BasicCamera::SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		mProjectionMatrix = BuildOrthographicMatrix(left, right, top, bottom, zNear, zFar);
	}

	void BasicCamera::Update(float deltaTime)
	{

	}

	bool BasicCamera::OnInputUpdate(EKeyCode keycode, EKeyState keyState, int xPos, int yPos)
	{
		if (keycode == KeyCode_MouseLeft && keyState == EKeyState::ePressed)
		{
			SG_LOG_DEBUG("Pressed!");
		}
		else if (keycode == KeyCode_MouseLeft && keyState == EKeyState::eRelease)
		{
			SG_LOG_DEBUG("Release!");
		}
		else if (keycode == KeyCode_MouseLeft && keyState == EKeyState::eHold)
		{
			SG_LOG_DEBUG("Holding!");
		}
		return true;
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