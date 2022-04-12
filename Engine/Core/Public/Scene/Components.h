#pragma once

#include "Math/MathBasic.h"

#include "TipECS/Registry.h"

namespace SG
{

	// temporary component
	// should be replaced by ECS

	//! Pure data + function class.
	class TransformComponent
	{
	public:
		TransformComponent()
			:mPosition(0.0f, 0.0f, 0.0f), mScale(1.0f, 1.0f, 1.0f), mRotation(0.0f, 0.0f, 0.0f)
		{}

		SG_INLINE void SetPosition(const Vector3f& pos) { mPosition = pos; }
		SG_INLINE Vector3f GetPosition() const { return mPosition; };

		SG_INLINE void SetScale(const Vector3f& scale) { mScale = scale; }
		SG_INLINE Vector3f GetScale()    const { return mScale; };

		SG_INLINE void SetRotation(const Vector3f& rotation) { mRotation = rotation; }
		SG_INLINE Vector3f GetRotation() const { return mRotation; };

		SG_INLINE Matrix4f GetTransform() const
		{
			return glm::translate(Matrix4f(1.0f), mPosition) *
				glm::toMat4(Quternion(glm::radians(mRotation))) *
				glm::scale(Matrix4f(1.0f), mScale);
		}
	private:
		Vector3f mPosition;
		Vector3f mScale;
		Vector3f mRotation;
	};

	//! Temporary, should use ECS for this.
	class MaterialComponent
	{
	public:
		SG_INLINE void     SetColor(const Vector3f& color) { mColor = color; }
		SG_INLINE Vector3f GetColor() const { return mColor; }

		SG_INLINE void  SetRoughness(float roughness) { mRoughness = roughness; }
		SG_INLINE float GetRoughness() const { return mRoughness; }

		SG_INLINE void  SetMetallic(float metallic) { mMetallic = metallic; }
		SG_INLINE float GetMetallic()  const { return mMetallic; }
	private:
		Vector3f mColor = { 1.0f, 1.0f, 1.0f };
		float    mMetallic = 0.7f;
		float    mRoughness = 0.35f;
	};

	using SGComponentList = TipECS::ComponentList<TransformComponent, MaterialComponent>;
	using SGTagList = TipECS::TagList<>;

	using TMSignature = TipECS::Signature<TransformComponent, MaterialComponent>;
	using SGSignatureList = TipECS::SignatureList<TMSignature>;

	using SGECSSetting = TipECS::Setting<SGComponentList, SGTagList, SGSignatureList>;

}