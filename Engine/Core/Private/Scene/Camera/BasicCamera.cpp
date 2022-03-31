#include "StdAfx.h"
#include "Scene/Camera/BasicCamera.h"

namespace SG
{

	BasicCamera::BasicCamera(const Vector3f& position, const Vector3f& rotation)
		:mPosition(position), mRotation(rotation), mbIsViewDirty(true), mbIsProjDirty(true)
	{
		Input::RegisterListener(EListenerPriority::eLevel2, this);
	}

	BasicCamera::~BasicCamera()
	{
		Input::RemoveListener(this);
	}

	void BasicCamera::SetPerspective(float fovyInDegrees, float aspect, float zNear, float zFar)
	{
		if (mbUseOrtho || fovyInDegrees != mFovyInDegrees || aspect != mAspect || zNear != mZNear || zFar != mZFar)
		{
			mbIsProjDirty = true;
			mbUseOrtho = false;
			mProjectionMatrix = BuildPerspectiveMatrix(glm::radians(fovyInDegrees), aspect, zNear, zFar);
			// inverse the proj matrix for vulkan's clip space coordinate
			mProjectionMatrix[1][1] *= -1.0f;

			mFovyInDegrees = fovyInDegrees;
			mAspect        = aspect;
			mZNear         = zNear;
			mZFar          = zFar;
		}
	}

	void BasicCamera::SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		static float sLeft = 0, sRight = 0, sTop = 0, sBottom = 0, sZNear = 0, sZFar = 0;

		if (!mbUseOrtho || left != sLeft || right != sRight || top != sTop || bottom != sBottom || zNear != sZNear || zFar != sZFar)
		{
			mbIsViewDirty = true;
			mbUseOrtho = true;
			mProjectionMatrix = BuildOrthographicMatrix(left, right, top, bottom, zNear, zFar);
			// inverse the proj matrix for vulkan's clip space coordinate
			mProjectionMatrix[1][1] *= -1.0f;

			sLeft   = left;
			sRight  = right;
			sTop    = top;
			sBottom = bottom;
			sZNear  = zNear;
			sZFar   = zFar;
		}
	}

}