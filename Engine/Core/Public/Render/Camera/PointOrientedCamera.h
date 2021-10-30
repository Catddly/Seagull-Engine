#pragma once

#include "Render/Camera/BasicCamera.h"

namespace SG
{

	class PointOrientedCamera final : public BasicCamera
	{
	public:
		SG_CORE_API PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt = Vector3f::Zero());
		SG_CORE_API ~PointOrientedCamera();
		SG_CLASS_NO_COPY_ASSIGNABLE(PointOrientedCamera);

		SG_CORE_API virtual void Update(float deltaTime) override;
	private:
		//SG_CORE_API virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;
		SG_CORE_API virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
		SG_CORE_API virtual bool OnMouseWheelInputUpdate(int direction) override;

		void UpdateViewMatrix();
	private:
		Vector3f mViewAtPoint;
		float    mMoveSpeed;
		float    mWheelScale;
	};

}