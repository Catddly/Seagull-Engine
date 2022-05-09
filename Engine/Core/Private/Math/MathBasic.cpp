#include "StdAfx.h"
#include "Math/MathBasic.h"

namespace glm
{

	void to_json(SG::json& node, const vec3& v)
	{
		node = { v.x, v.y, v.z };
	}

	void from_json(const SG::json& node, vec3& v)
	{
		node.at(0).get_to(v.x);
		node.at(1).get_to(v.y);
		node.at(2).get_to(v.z);
	}

}