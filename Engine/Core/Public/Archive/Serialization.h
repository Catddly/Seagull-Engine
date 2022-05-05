#pragma once

#include "Scene/Scene.h"
#include "Archive/ISerializable.h"

#include "Stl/SmartPtr.h"

namespace SG
{

	class Serializer
	{
	public:
		static void Serialize(RefPtr<Scene> pScene);
	private:

	};

	class Deserializer
	{
	public:
		static void Deserialize(RefPtr<Scene> pScene);
	private:

	};

}