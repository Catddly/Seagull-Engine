#pragma once

#include "Scene/Camera/BasicCamera.h"

namespace SG
{

	class PointOrientedCamera final : public BasicCamera
	{
	public:
		SG_CORE_API PointOrientedCamera(const Vector3f& pos, const Vector3f& viewAt = Vector3f(0.0f));
		SG_CORE_API ~PointOrientedCamera();
		SG_CLASS_NO_COPY_ASSIGNABLE(PointOrientedCamera);
	private:
		SG_CORE_API virtual bool OnMouseMoveInputUpdate(int xPos, int yPos, int deltaXPos, int deltaYPos) override;
		SG_CORE_API virtual bool OnMouseWheelInputUpdate(int direction) override;

		virtual void UpdateViewMatrix() override;
	private:
		Vector3f mViewAtPoint;
		float    mMoveSpeed;
		float    mWheelScale;
	};

}