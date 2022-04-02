#pragma once

#include "Core/Config.h"
#include "Defs/Defs.h"

#include "Math/MathBasic.h"
#include "Math/Frustum.h"

namespace SG
{

	interface SG_CORE_API ICamera
	{
		virtual ~ICamera() = default;

		virtual Matrix4f GetViewMatrix() const = 0;
		virtual Matrix4f GetProjMatrix() const = 0;

		virtual void SetPerspective(float fovyInDegrees, float aspect, float zNear = 0.01f, float zFar = 256.0f) = 0;
		virtual void SetOrthographic(float left, float right, float top, float bottom, float zNear, float zFar) = 0;

		virtual void     SetPosition(const Vector3f& pos) = 0;
		virtual Vector3f GetPosition() const = 0;
		virtual void     SetRotation(const Vector3f& rot) = 0;
		virtual Vector3f GetRotation() const = 0;

		virtual Frustum GetFrustum() const = 0;

		virtual bool IsViewDirty() const = 0;
		virtual bool IsProjDirty() const = 0;

		virtual void ViewBeUpdated() = 0;
		virtual void ProjBeUpdated() = 0;

		virtual void OnUpdate(float deltaTime) = 0;
	};

}