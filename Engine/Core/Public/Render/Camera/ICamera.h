#pragma once

#include "Core/Config.h"
#include "Defs/Defs.h"

#include "Math/Matrix.h"
#include "Math/Vector.h"

namespace SG
{

	interface SG_CORE_API Camera
	{
		virtual ~Camera() = default;

		virtual Matrix4f GetProjMatrix() const = 0;
		virtual Matrix4f GetViewMatrix() const = 0;

		virtual void SetPosition(const Vector3f& pos) = 0;
		virtual Vector3f GetPosition() const = 0;

		virtual void Update(float deltaTime) = 0;
	};

}