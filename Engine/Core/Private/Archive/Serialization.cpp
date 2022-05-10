#include "StdAfx.h"
#include "Archive/Serialization.h"

#include "System/FileSystem.h"
#include "Profile/Profile.h"

namespace SG
{

	void Serializer::Serialize(RefPtr<Scene> pScene, const char* sceneName)
	{
		SG_PROFILE_FUNCTION();
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);

		ISerializable* pSerializable = pScene.get();

		json newNode;
		pSerializable->Serialize(newNode);

		if (FileSystem::Open(EResourceDirectory::eScenes, sceneName, EFileMode::efWrite, SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			string dumpStr = newNode.dump(4).c_str();
			FileSystem::Write(dumpStr.c_str(), dumpStr.size() * sizeof(char));
			FileSystem::Close();
		}
	}

	void Deserializer::Deserialize(RefPtr<Scene> pScene, const char* sceneName)
	{
		SG_PROFILE_FUNCTION();
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);

		if (FileSystem::Exist(EResourceDirectory::eScenes, sceneName, SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			string jsonStr = FileSystem::ReadWholeFileAsText(EResourceDirectory::eScenes, sceneName, SG_ENGINE_DEBUG_BASE_OFFSET);

			auto node = json::parse(jsonStr.c_str());

			ISerializable* pSerializable = pScene.get();
			pSerializable->Deserialize(node);
		}
		else
		{
			SG_LOG_ERROR("Failed to load %s!", sceneName);
		}
	}

}