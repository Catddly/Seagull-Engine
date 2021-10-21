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
		SG_CORE_API virtual bool OnInputUpdate(EKeyCode keycode, EKeyState keyState, int xPos, int yPos) override;
	private:
		Vector3f mViewAtPoint;
	};

}