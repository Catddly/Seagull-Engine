#pragma once

#include "System/Input.h"
#include "Scene/Camera/ICamera.h"

#include "Math/MathBasic.h"

namespace SG
{

	class BasicCamera : public ICamera, protected IInputListener
	{
	public:
		SG_CORE_API BasicCamera(const Vector3f& position, const Vector3f& rotation);
		SG_CORE_API ~BasicCamera();
		SG_CLASS_NO_COPY_MOVE_ASSIGNABLE(BasicCamera);

		SG_CORE_API virtual Matrix4f GetViewMatrix() const override { return mViewMatrix; }
		SG_CORE_API virtual Matrix4f GetProjMatrix() const override { return mProjectionMatrix; }

		SG_CORE_API virtual void SetPerspective(float fovyInDegrees, float aspect, float zNear = 0.01f, float zFar = 256.0f) override;
		SG_CORE_API virtual void SetOrthographic(float left, float right, float top, float bottom, float zNear, float zfar) override;

		SG_CORE_API virtual void     SetPosition(const Vector3f& pos) override { mbIsViewDirty = true; mPosition = pos; UpdateViewMatrix(); }
		SG_CORE_API virtual Vector3f GetPosition() const override { return mPosition; }
		SG_CORE_API virtual void     SetRotation(const Vector3f& rot) override { mbIsViewDirty = true; mRotation = rot; UpdateViewMatrix(); }
		SG_CORE_API virtual Vector3f GetRotation() const override { return mRotation; }

		SG_CORE_API virtual bool IsViewDirty() const override { return mbIsViewDirty; }
		SG_CORE_API virtual bool IsProjDirty() const override { return mbIsProjDirty; }

		SG_CORE_API virtual Frustum GetFrustum() const override;

		SG_CORE_API virtual void ViewBeUpdated() override { mbIsViewDirty = false; }
		SG_CORE_API virtual void ProjBeUpdated() override { mbIsProjDirty = false; }

		SG_CORE_API virtual void OnUpdate(float deltaTime) { mDeltaTime = deltaTime; }
	protected:
		void CalcFrustum();
		SG_CORE_API virtual void UpdateViewMatrix() {}
	protected:
		float    mDeltaTime = 0.0f;
		Vector3f mPosition;
		float    mAspectRatio;
		Vector3f mRotation;

		Matrix4f mViewMatrix;
		Matrix4f mProjectionMatrix;

		Frustum mFrustum;

		mutable bool mbIsViewDirty = false;
		mutable bool mbIsProjDirty = false;
		bool mbUseOrtho = false;
	};

}