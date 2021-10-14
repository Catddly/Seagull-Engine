#pragma once

#include "Render/Camera/ICamera.h"

#include "System/IInput.h"

#include "Math/Vector.h"
#include "Math/Matrix.h"

namespace SG
{

	class PointOrientedCamera : public Camera, protected IInputListener
	{
	public:
		SG_CORE_API PointOrientedCamera();
		SG_CORE_API PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt = Vector3f::Zero());
		SG_CORE_API ~PointOrientedCamera();
		SG_CLASS_NO_COPY_ASSIGNABLE(PointOrientedCamera);

		SG_CORE_API virtual Matrix4f GetProjMatrix() const override { return Matrix4f::Identity(); }
		SG_CORE_API virtual Matrix4f GetViewMatrix() const override { return mViewMatrix; }

		SG_CORE_API virtual void SetPosition(const Vector3f& pos) { mPosition = pos; CalcViewMatrix(); }
		SG_CORE_API virtual Vector3f GetPosition() const override { return mPosition; }

		SG_CORE_API virtual void Update(float deltaTime) override {}
	private:
		SG_CORE_API void CalcViewMatrix();

		virtual bool OnInputUpdate(EKeyCode keycode, EKeyState keyState) override;
	private:
		Vector3f mPosition;
		Vector3f mViewAtPoint;
		Matrix4f mViewMatrix;
	};

}