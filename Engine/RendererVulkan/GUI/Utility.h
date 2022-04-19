#pragma once

#include "Math/MathBasic.h"
#include "Scene/Scene.h"

#include "Stl/string.h"

namespace SG
{

	bool DrawEntityProperty(Scene::Entity& entity);

	bool DrawGUIDragFloat(const string& label, float& values, float defaultValue = 0.0f, float speed = 0.05f,
		float minValue = 0.0f, float maxValue = 0.0f, float columnWidth = 100.0f);

	bool DrawGUIDragFloat3(const string& label, Vector3f& values, const Vector3f& defaultValue = Vector3f(0.0f), float speed = 0.05f, 
		const Vector3f& minValues = Vector3f(0.0f), const Vector3f& maxValues = Vector3f(0.0f), float columnWidth = 100.0f);

	bool DrawGUIColorEdit3(const string& label, Vector3f& values, const Vector3f& defaultValue = { 1.0f, 1.0f, 1.0f }, float columnWidth = 100.0f);

}