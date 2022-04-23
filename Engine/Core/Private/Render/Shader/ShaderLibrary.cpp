#include "StdAfx.h"
#include "Render/Shader/ShaderLibrary.h"

#include "System/Logger.h"
#include "System/FileSystem.h"
#include "Profile/Profile.h"

#include "Stl/vector.h"

namespace SG
{

	void ShaderLibrary::OnInit()
	{
		SG_PROFILE_FUNCTION();

		// if can't find shader_env.ini, just keep the map empty.
		if (FileSystem::Open(EResourceDirectory::eRoot, "shader_env.ini", EFileMode::efRead))
		{
			string str;
			str.resize(FileSystem::FileSize());
			FileSystem::Read(str.data(), str.size() * sizeof(char));

			Size leftPos = 0;
			Size rightPos = 0;

			do 
			{
				leftPos = str.find_first_of('[', leftPos);
				if (leftPos == string::npos)
					break;
				rightPos = str.find_first_of(']', rightPos + 1);
				string name = str.substr(leftPos + 1, rightPos - leftPos - 1);
				//SG_LOG_DEBUG("%s", name.c_str());

				leftPos = str.find_first_of('[', leftPos + 1);
				rightPos = str.find_first_of(':', rightPos + 1);
				unsigned long year = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				leftPos = rightPos;
				rightPos = str.find_first_of(':', rightPos + 1);
				unsigned long month = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				leftPos = rightPos;
				rightPos = str.find_first_of(']', rightPos + 1);
				unsigned long day = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				leftPos = str.find_first_of('[', leftPos + 1);
				rightPos = str.find_first_of(':', rightPos + 1);
				unsigned long hour = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				leftPos = rightPos;
				rightPos = str.find_first_of(':', rightPos + 1);
				unsigned long minute = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				leftPos = rightPos;
				rightPos = str.find_first_of(']', rightPos + 1);
				unsigned long second = std::strtoul(str.substr(leftPos + 1, rightPos - leftPos - 1).c_str(), nullptr, 0);

				TimePoint tp = {};
				tp.year = static_cast<UInt16>(year);
				tp.month = static_cast<UInt16>(month);
				tp.day = static_cast<UInt16>(day);
				tp.hour = static_cast<UInt16>(hour);
				tp.minute = static_cast<UInt16>(minute);
				tp.second = static_cast<UInt16>(second);

				mShaderTimePointMap[name] = eastl::move(tp);
			} while (true);

			FileSystem::Close();
		}
	}

	void ShaderLibrary::OnShutdown()
	{
		SG_PROFILE_FUNCTION();

		if (FileSystem::Open(EResourceDirectory::eRoot, "shader_env.ini", EFileMode::efWrite))
		{
			FileSystem::TraverseFileAndSubDirectoryInFolder(EResourceDirectory::eShader_Sources, "", [](const char* filename)
				{
					// filename
					string writeStr = "[";
					writeStr += filename;
					writeStr += "]\n";
					// last write time data
					TimePoint tp = FileSystem::GetFileLastWriteTime(EResourceDirectory::eShader_Sources, filename, SG_ENGINE_DEBUG_BASE_OFFSET);
					writeStr += "[" +
						eastl::to_string(tp.year) + ":" +
						eastl::to_string(tp.month) + ":" +
						eastl::to_string(tp.day) + "] [" +
						eastl::to_string(tp.hour) + ":" +
						eastl::to_string(tp.minute) + ":" +
						eastl::to_string(tp.second) + "]\n";

					FileSystem::Write(writeStr.data(), writeStr.size() * sizeof(char));
				}, SG_ENGINE_DEBUG_BASE_OFFSET);
			FileSystem::Close();
		}
	}

	void ShaderLibrary::SetShaderTimeStamp(const string& shaderName, const TimePoint& tp)
	{
		SG_PROFILE_FUNCTION();

		auto pNode = mShaderTimePointMap.find(shaderName);
		if (pNode == mShaderTimePointMap.end())
		{
			SG_LOG_ERROR("Failed to find the time stamp of shader: %s", shaderName);
			return;
		}
		mShaderTimePointMap[shaderName] = tp;
	}

	TimePoint ShaderLibrary::GetShaderTimeStamp(const string& shaderName)
	{
		SG_PROFILE_FUNCTION();

		auto pNode = mShaderTimePointMap.find(shaderName);
		if (pNode == mShaderTimePointMap.end())
		{
			//SG_LOG_ERROR("Failed to find the time stamp of shader: %s", shaderName);
			return TimePoint();
		}
		return pNode->second;
	}

	ShaderLibrary* ShaderLibrary::GetInstance()
	{
		static ShaderLibrary sInstance;
		return &sInstance;
	}

}