#include "StdAfx.h"
#include "Math/BoundingBox.h"

namespace SG
{

	void to_json(json& node, const BoundingBox& v)
	{
		node = {
			v.minBound.x, v.minBound.y, v.minBound.z,
			v.maxBound.x, v.maxBound.y, v.maxBound.z
		};
	}

	void from_json(const json& node, BoundingBox& v)
	{
		node.at(0).get_to(v.minBound.x);
		node.at(1).get_to(v.minBound.y);
		node.at(2).get_to(v.minBound.z);
		node.at(3).get_to(v.maxBound.x);
		node.at(4).get_to(v.maxBound.y);
		node.at(5).get_to(v.maxBound.z);
	}

}