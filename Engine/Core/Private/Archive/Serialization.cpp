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

		using nlohmann::json;

		json serializer;
		serializer["Hello"] = 2;
		serializer["EntityID"]["Name"] = "ILLs";
		serializer["EntityID"]["Age"] = 9;
		serializer["EntityID"]["Hair Length"] = 33;

		SG_LOG_DEBUG("%s", serializer.dump().c_str());

		//ISerializable* pSerializable = pScene.get();

		//UInt32 counter = 0;
		//string subSceneName = "default.sub_0.scene";

		//// clear all the old data
		//while (FileSystem::Exist(EResourceDirectory::eScenes, subSceneName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
		//{
		//	bool res = FileSystem::RemoveFile(EResourceDirectory::eScenes, subSceneName, SG_ENGINE_DEBUG_BASE_OFFSET);
		//	++counter;
		//	subSceneName = "default.sub_" + eastl::to_string(counter) + ".scene";
		//}

		//// reset
		//counter = 0;
		//subSceneName = "default.sub_0.scene";

		//bool res = true;
		//do
		//{
		//	YAML::Emitter out;
		//	res = pSerializable->Serialize(out);

		//	if (out.good() && FileSystem::Open(EResourceDirectory::eScenes, subSceneName.c_str(), EFileMode::efWrite, SG_ENGINE_DEBUG_BASE_OFFSET))
		//	{
		//		FileSystem::Write(out.c_str(), out.size() * sizeof(char));
		//		FileSystem::Close();
		//	}

		//	++counter;
		//	subSceneName = "default.sub_" + eastl::to_string(counter) + ".scene";
		//} while (!res);
	}

	void Deserializer::Deserialize(RefPtr<Scene> pScene)
	{
		SG_PROFILE_FUNCTION();
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);

		//UInt32 counter = 0;
		//string subSceneName = "default.sub_0.scene";

		//while (FileSystem::Exist(EResourceDirectory::eScenes, subSceneName.c_str(), SG_ENGINE_DEBUG_BASE_OFFSET))
		//{
		//	string buf;

		//	if (FileSystem::Open(EResourceDirectory::eScenes, subSceneName.c_str(), EFileMode::efRead, SG_ENGINE_DEBUG_BASE_OFFSET))
		//	{
		//		buf.resize(FileSystem::FileSize());
		//		FileSystem::Read(buf.data(), FileSystem::FileSize());
		//		FileSystem::Close();
		//	}

		//	//try
		//	//{
		//	YAML::Node data = YAML::Load(buf.c_str());
		//	ISerializable* pSerializable = pScene.get();
		//	pSerializable->Deserialize(data);
		//	//}
		//	//catch (YAML::TypedBadConversion<float> e)
		//	//{
		//	//	SG_LOG_DEBUG("%s", e.what());
		//	//}

		//	++counter;
		//	subSceneName = "default.sub_" + eastl::to_string(counter) + ".scene";
		//}
	}

}