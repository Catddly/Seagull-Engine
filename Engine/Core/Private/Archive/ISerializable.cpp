#include "StdAfx.h"
#include "Archive/ISerializable.h"

namespace SG
{

	YAML::Emitter& operator<<(YAML::Emitter& out, const Vector3f& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

}