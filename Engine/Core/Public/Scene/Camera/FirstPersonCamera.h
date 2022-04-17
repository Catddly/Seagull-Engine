#pragma once

#include "Scene/Camera/BasicCamera.h"

namespace SG
{

	class FirstPersonCamera final : public BasicCamera
	{
	public:
		FirstPersonCamera(const Vector3f& position);
		~FirstPersonCamera();
		SG_CLASS_NO_COPY_ASSIGNABLE(FirstPersonCamera);

		virtual bool OnKeyInputUpdate(EKeyCode keycode, EKeyState keyState) override;
		virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;

		virtual void UpdateViewMatrix() override;
	private:
		Vector3f mUpVec = SG_ENGINE_UP_VEC();
		Vector3f mFrontVec = SG_ENGINE_FRONT_VEC();
		Vector3f mRightVec = SG_ENGINE_RIGHT_VEC();

		float mMoveSpeed = 5.0f;
		float mRotateSpeed = 1.5f;
	};

}