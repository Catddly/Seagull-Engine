#include "StdAfx.h"
#include "Archive/Serialization.h"

#include "System/FileSystem.h"

namespace SG
{

	void Serializer::Serialize(RefPtr<Scene> pScene)
	{
		YAML::Emitter out;
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);
		ISerializable* pSerializable = pScene.get();
		pSerializable->Serialize(out);

		if (FileSystem::Open(EResourceDirectory::eScenes, "default.scene", EFileMode::efWrite, SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			FileSystem::Write(out.c_str(), out.size());
			FileSystem::Close();
		}
	}

	void Deserializer::Deserialize(RefPtr<Scene> pScene)
	{
		string readBuf;
		if (FileSystem::Open(EResourceDirectory::eScenes, "default.scene", EFileMode::efRead, SG_ENGINE_DEBUG_BASE_OFFSET))
		{
			readBuf.resize(FileSystem::FileSize());
			FileSystem::Read(readBuf.data(), readBuf.size());
			FileSystem::Close();
		}

		SG_ASSERT(!readBuf.empty());

		YAML::Node data = YAML::Load(readBuf.c_str());
		SG_COMPILE_ASSERT(eastl::is_base_of<ISerializable, Scene>::value);
		ISerializable* pSerializable = pScene.get();
		pSerializable->Deserialize(data);
	}

}