#pragma once

#include "Core/Config.h"
#include "Scene/Scene.h"
#include "Archive/ISerializable.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class Serializer
	{
	public:
		SG_CORE_API static void Serialize(RefPtr<Scene> pScene, const char* sceneName);
	private:

	};

	class Deserializer
	{
	public:
		SG_CORE_API static void Deserialize(RefPtr<Scene> pScene, const char* sceneName);
	private:

	};

}