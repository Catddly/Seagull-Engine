#pragma once

#include "Defs/Defs.h"

#include "nlohmann/json.hpp"

namespace SG
{

	using json = nlohmann::json;

	interface ISerializable
	{
		virtual ~ISerializable() = default;

		virtual void Serialize(json& node) = 0;
		virtual void Deserialize(json& node) = 0;
	};

}