#pragma once

#include "Base/TimePoint.h"

#include <eastl/unordered_map.h>

namespace SG
{

	//! Singleton class.
	class ShaderLibrary
	{
	public:
		~ShaderLibrary() = default;

		void      SetShaderTimeStamp(const string& shaderName, const TimePoint& tp);
		TimePoint GetShaderTimeStamp(const string& shaderName);

		static ShaderLibrary* GetInstance();
	private:
		ShaderLibrary() = default;
		friend class System;

		//! Read in shader_env.ini.
		void OnInit();
		//! Write shader_env.ini.
		void OnShutdown();
	private:
		eastl::unordered_map<string, TimePoint> mShaderTimePointMap;
	};

}