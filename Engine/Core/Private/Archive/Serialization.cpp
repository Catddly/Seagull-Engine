#include "StdAfx.h"
#include "Archive/Serialization.h"

#include "System/FileSystem.h"
#include "Profile/Profile.h"

namespace SG
{

	void Serializer::Serialize(RefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);

		// clear old data
		//if (FileSystem::Exist(EResourceDirectory::eScenes, sceneName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
		//	FileSystem::RemoveFile(EResourceDirectory::eScenes, sceneName, SG_ENGINE_DEBUG_BASE_OFFSET);

		ISerializable* pSerializable = pScene.get();

		json newNode;
		pSerializable->Serialize(newNode);

		if (FileSystem::Open(EResourceDirectory::eScenes, "default.scene", EFileMode::efWrite, SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			string dumpStr = newNode.dump(4).c_str();
			FileSystem::Write(dumpStr.c_str(), dumpStr.size() * sizeof(char));
			FileSystem::Close();
		}
	}

	void Deserializer::Deserialize(RefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);

		if (FileSystem::Exist(EResourceDirectory::eScenes, "default.scene", SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			string jsonStr = FileSystem::ReadWholeFileAsText(EResourceDirectory::eScenes, "default.scene", SG_ENGINE_DEBUG_BASE_OFFSET);

			auto node = json::parse(jsonStr.c_str());

			ISerializable* pSerializable = pScene.get();
			pSerializable->Deserialize(node);
		}
		else
		{
			SG_LOG_ERROR("Failed to load default.scene!");
		}
	}

}