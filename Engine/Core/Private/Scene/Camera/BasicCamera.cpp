#include "StdAfx.h"
#include "Scene/Camera/BasicCamera.h"

namespace SG
{

	static float gAspect;

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
		static float sFovyInDegrees = 0, sAspect = 0, sZNear = 0, sZFar = 0;

		if (mbUseOrtho || fovyInDegrees != sFovyInDegrees || aspect != sAspect || zNear != sZNear || zFar != sZFar)
		{
			gAspect = aspect;

			mbIsProjDirty = true;
			mbUseOrtho = false;
			mProjectionMatrix = BuildPerspectiveMatrix(glm::radians(fovyInDegrees), aspect, zNear, zFar);
			// inverse the proj matrix for vulkan's clip space coordinate
			mProjectionMatrix[1][1] *= -1.0f;

			sFovyInDegrees = fovyInDegrees;
			sAspect        = aspect;
			sZNear         = zNear;
			sZFar          = zFar;

			CalcFrustum();
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

			CalcFrustum();
		}
	}

	Frustum BasicCamera::GetFrustum() const
	{
		return mFrustum;
	}

	void BasicCamera::CalcFrustum()
	{
		// for test, force set the fovy to 20.f
		Matrix4f proj = BuildPerspectiveMatrix(glm::radians(25.0f), gAspect, 0.01f, 256.0f);
		// fast way to build a frustum
		Matrix4f viewProj = proj * mViewMatrix;

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

}