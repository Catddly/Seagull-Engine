#pragma once

#include "Defs/Defs.h"

#include "Math/MathBasic.h"

#include "nlohmann/json.hpp"

namespace SG
{

	interface ISerializable
	{
		virtual ~ISerializable() = default;

		virtual void Serialize() = 0;
		virtual void Deserialize() = 0;
	};

}