#pragma once

#include "Math/Vector.h"

namespace SG
{

	class PointLight
	{
	public:
		explicit PointLight(Vector3f position, float radius, Vector3f color)
			:mPosition(position), mRadius(radius), mColor(color)
		{}
		~PointLight() = default;

		void     SetPosition(Vector3f pos) { mPosition = pos; mIsDirty = true; }
		Vector3f GetPosition()       const { return mPosition; }

		void  SetRadius(float radius) { mRadius = radius; mIsDirty = true; }
		float GetRadius()       const { return mRadius; }

		void     SetColor(Vector3f color) { mColor = color; mIsDirty = true; }
		Vector3f GetColor()         const { return mColor; }

		bool IsDirty() const { return mIsDirty; }
		void BeUpdated() const { mIsDirty = false; }
	private:
		Vector3f mPosition;
		float    mRadius;
		Vector3f mColor;
		mutable bool mIsDirty = true;
	};

}