#pragma once

#include "Base/BasicTypes.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h" // overload global new and delete

#include "Stl/string.h"

#include <eigen/Dense>

#include "Math/MathBasic.h"

namespace SG
{
	//typedef Eigen::Vector2f Vector2f;
	//typedef Eigen::Vector3f Vector3f;
	//typedef Eigen::Vector4f Vector4f;

	//typedef Eigen::Vector2i Vector2i;
	//typedef Eigen::Vector3i Vector3i;
	//typedef Eigen::Vector4i Vector4i;

	// overload log out functions
	namespace impl
	{
		template <typename T>
		static void _PrintVector(string& s, UInt32 width, UInt32 height, const T& value)
		{
			Float32 maxNum = Float32(value.maxCoeff() > Abs(value.minCoeff()) ? value.maxCoeff() : value.minCoeff());
			eastl::string num = eastl::to_string(maxNum);
			UInt32 maxBit = (UInt32)num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (value.minCoeff() < 0)
				bHasNegative = true;

			for (UInt32 i = 0; i < width; i++)
			{
				s += "[ ";
				for (UInt32 j = 0; j < height; j++)
				{
					UInt32 tempBit = maxBit + 1;
					if (value(i, j) > 0 && bHasNegative)
					{
						s += ' ';
						tempBit -= 1;
					}
					eastl::string num = eastl::to_string(value(i, j));
					eastl::string clipedNum = num.substr(0, num.find_first_of('.') + 2);
					tempBit -= (UInt32)clipedNum.size();
					s += clipedNum;
					for (UInt32 k = 0; k < tempBit; k++)
						s += ' ';
				}
				s += ']';
				if (i != width - 1)
					s += '\n';
			}
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector2f>(const SG::Vector2f& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector2f:\n";

			_PrintVector(s, 2, 1, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector3f>(const SG::Vector3f& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector3f:\n";

			_PrintVector(s, 3, 1, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector4f>(const SG::Vector4f& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector4f:\n";

			_PrintVector(s, 4, 1, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector2i>(const SG::Vector2i& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector2i:\n";

			_PrintVector(s, 2, 1, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector3i>(const SG::Vector3i& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector3i:\n";

			_PrintVector(s, 3, 1, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<SG::Vector4i>(const SG::Vector4i& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Vector4i:\n";

			_PrintVector(s, 4, 1, types);
			return eastl::move(s);
		}
	}


}