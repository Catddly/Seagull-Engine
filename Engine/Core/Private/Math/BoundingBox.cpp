#include "StdAfx.h"
#include "Math/BoundingBox.h"

namespace SG
{

	void to_json(json& node, const AABB& v)
	{
		node = {
			v.min.x, v.min.y, v.min.z,
			v.max.x, v.max.y, v.max.z
		};
	}

	void from_json(const json& node, AABB& v)
	{
		node.at(0).get_to(v.min.x);
		node.at(1).get_to(v.min.y);
		node.at(2).get_to(v.min.z);
		node.at(3).get_to(v.max.x);
		node.at(4).get_to(v.max.y);
		node.at(5).get_to(v.max.z);
	}

}