#include "StdAfx.h"
#include "Scene/Camera/BasicCamera.h"

#include "System/Logger.h"
#include "Profile/Profile.h"

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
		SG_PROFILE_FUNCTION();

		static float sFovyInDegrees = 0, sZNear = 0, sZFar = 0;

		if (mbUseOrtho || fovyInDegrees != sFovyInDegrees || aspect != mAspectRatio || zNear != sZNear || zFar != sZFar)
		{
			mbIsProjDirty = true;
			mbUseOrtho = false;
			mProjectionMatrix = BuildPerspectiveMatrix(glm::radians(fovyInDegrees), aspect, zNear, zFar);

			sFovyInDegrees = fovyInDegrees;
			mAspectRatio   = aspect;
			sZNear         = zNear;
			sZFar          = zFar;

			CalcFrustum();
			CalcFrustumBoundingBox();
		}
	}

	void BasicCamera::SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar)
	{
		SG_PROFILE_FUNCTION();

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

			CalcFrustum();
			CalcFrustumBoundingBox();
		}
	}

	Frustum BasicCamera::GetFrustum() const
	{
		return mFrustum;
	}

	BoundingBox BasicCamera::GetFrustumBoundingBox() const
	{
		return mFrustumBoundingBox;
	}

	void BasicCamera::CalcFrustum()
	{
		SG_PROFILE_FUNCTION();

		Matrix4f viewProj = mProjectionMatrix * mViewMatrix;

		// fast way to build a frustum
		Plane topPlane({
			viewProj[0].w - viewProj[0].y,
			viewProj[1].w - viewProj[1].y,
			viewProj[2].w - viewProj[2].y,
			viewProj[3].w - viewProj[3].y });
		topPlane /= Sqrt(glm::dot(Vector3f(Vector4f(topPlane)), Vector3f(Vector4f(topPlane))));
		Plane bottomPlane({
			viewProj[0].w + viewProj[0].y,
			viewProj[1].w + viewProj[1].y,
			viewProj[2].w + viewProj[2].y,
			viewProj[3].w + viewProj[3].y });
		bottomPlane /= Sqrt(glm::dot(Vector3f(Vector4f(bottomPlane)), Vector3f(Vector4f(bottomPlane))));
		Plane rightPlane({
			viewProj[0].w - viewProj[0].x,
			viewProj[1].w - viewProj[1].x,
			viewProj[2].w - viewProj[2].x,
			viewProj[3].w - viewProj[3].x });
		rightPlane /= Sqrt(glm::dot(Vector3f(Vector4f(rightPlane)), Vector3f(Vector4f(rightPlane))));
		Plane leftPlane({
			viewProj[0].w + viewProj[0].x,
			viewProj[1].w + viewProj[1].x,
			viewProj[2].w + viewProj[2].x,
			viewProj[3].w + viewProj[3].x });
		leftPlane /= Sqrt(glm::dot(Vector3f(Vector4f(leftPlane)), Vector3f(Vector4f(leftPlane))));
		Plane frontPlane({
			viewProj[0].w - viewProj[0].z,
			viewProj[1].w - viewProj[1].z,
			viewProj[2].w - viewProj[2].z,
			viewProj[3].w - viewProj[3].z });
		frontPlane /= Sqrt(glm::dot(Vector3f(Vector4f(frontPlane)), Vector3f(Vector4f(frontPlane))));
		Plane backPlane({
			viewProj[0].w + viewProj[0].z,
			viewProj[1].w + viewProj[1].z,
			viewProj[2].w + viewProj[2].z,
			viewProj[3].w + viewProj[3].z });
		backPlane /= Sqrt(glm::dot(Vector3f(Vector4f(backPlane)), Vector3f(Vector4f(backPlane))));
		mFrustum = { frontPlane, backPlane, rightPlane, leftPlane, topPlane, bottomPlane };
	}

	void BasicCamera::CalcFrustumBoundingBox()
	{
		SG_PROFILE_FUNCTION();

		constexpr const int NUM_CORNER_POINTS = 8;

		const Vector3f frustumPointsNDCSpace[NUM_CORNER_POINTS] = {
			Vector3f(-1.0f, -1.0f, 1.0f),
			Vector3f( 1.0f, -1.0f, 1.0f),
			Vector3f( 1.0f,  1.0f, 1.0f),
			Vector3f(-1.0f,  1.0f, 1.0f),
			Vector3f(-1.0f, -1.0f, 0.0f),
			Vector3f( 1.0f, -1.0f, 0.0f),
			Vector3f( 1.0f,  1.0f, 0.0f),
			Vector3f(-1.0f,  1.0f, 0.0f) 
		};

		Matrix4f inverseVPMat = glm::inverse(mProjectionMatrix * mViewMatrix);
		BBoxReset(mFrustumBoundingBox);

		for (int i = 0; i < NUM_CORNER_POINTS; ++i)
		{
			const Vector4f frustumCornerPointWithW = inverseVPMat * Vector4f{ frustumPointsNDCSpace[i].x, frustumPointsNDCSpace[i].y, frustumPointsNDCSpace[i].z, 1.0f};
			// normalized
			const Vector3f frustumCornerPointWorldSpace = {
				frustumCornerPointWithW.x / frustumCornerPointWithW.w,
				frustumCornerPointWithW.y / frustumCornerPointWithW.w,
				frustumCornerPointWithW.z / frustumCornerPointWithW.w
			};

			BBoxMerge(mFrustumBoundingBox, frustumCornerPointWorldSpace);
		}
	}

}