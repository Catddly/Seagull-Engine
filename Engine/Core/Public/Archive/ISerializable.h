#pragma once

#include "Defs/Defs.h"

#include "Math/MathBasic.h"

// use yaml as the scene save file format
#include "yaml-cpp/yaml.h"

// serialize custom data structures
namespace YAML 
{

	template<>
	struct convert<SG::Vector3f>
	{
		static Node encode(const SG::Vector3f& v)
		{
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			return node;
		}

		static bool decode(const Node& node, SG::Vector3f& v)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();
			return true;
		}
	};

}

namespace SG
{

	YAML::Emitter& operator<<(YAML::Emitter& out, const SG::Vector3f& v);

	interface ISerializable
	{
		virtual ~ISerializable() = default;

		virtual void Serialize(YAML::Emitter& outStream) = 0;
		virtual void Deserialize(YAML::Node& node) = 0;
	};

}