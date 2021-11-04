#pragma once

#include "System/Input.h"
#include "Render/Camera/ICamera.h"

#include "Math/Vector.h"
#include "Math/Matrix.h"

namespace SG
{

	class BasicCamera : public ICamera, protected IInputListener
	{
	public:
		SG_CORE_API BasicCamera(const Vector3f& position, const Vector3f& rotation);
		SG_CORE_API ~BasicCamera();
		SG_CLASS_NO_COPY_ASSIGNABLE(BasicCamera);

		SG_CORE_API virtual Matrix4f GetViewMatrix() const override { mbIsViewDirty = false; return mViewMatrix; }
		SG_CORE_API virtual Matrix4f GetProjMatrix() const override { mbIsProjDirty = false; return mProjectionMatrix; }

		SG_CORE_API virtual void SetPerspective(float fovyInDegrees, float aspect, float zNear = 0.001f, float zFar = 1000.0f) override;
		SG_CORE_API virtual void SetOrthographic(float left, float right, float top, float bottom, float zNear, float zfar) override;

		SG_CORE_API virtual void     SetPosition(const Vector3f& pos) override { mbIsViewDirty = false; mPosition = pos; UpdateViewMatrix(); }
		SG_CORE_API virtual Vector3f GetPosition() const override { return mPosition; }
		SG_CORE_API virtual void     SetRotation(const Vector3f& rot) override { mbIsViewDirty = false; mRotation = rot; UpdateViewMatrix(); }
		SG_CORE_API virtual Vector3f GetRotation() const override { return mRotation; }

		SG_CORE_API virtual bool     IsViewDirty() const override { return mbIsViewDirty; }
		SG_CORE_API virtual bool     IsProjDirty() const override { return mbIsProjDirty; }

		SG_CORE_API virtual void Update(float deltaTime) override {}
	protected:
		SG_CORE_API void UpdateViewMatrix();
	protected:
		Vector3f mPosition;
		Vector3f mRotation;

		Matrix4f mViewMatrix;
		Matrix4f mProjectionMatrix;

		mutable bool mbIsViewDirty = false;
		mutable bool mbIsProjDirty = false;
		bool mbUseOrtho = false;
	};

}