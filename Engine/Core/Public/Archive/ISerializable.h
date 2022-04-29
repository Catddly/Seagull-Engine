#pragma once

#include "Defs/Defs.h"

namespace SG
{

	interface ISerializable
	{
		virtual ~ISerializable() = default;

		virtual void Serialize() = 0;
		virtual void DeSerialize() = 0;
	};

}