#pragma once

#include "Base/BasicTypes.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h" // overload global new and delete

#include <eigen/Dense>

#include "Math/MathBasic.h"

namespace SG
{
	typedef Eigen::Matrix<Float32, 3, 3> Matrix3f;
	typedef Eigen::Matrix<Float32, 4, 4> Matrix4f;

	typedef Eigen::Matrix<Int32, 3, 3>   Matrix3i;
	typedef Eigen::Matrix<Int32, 4, 4>   Matrix4i;

	// overload log out function
	namespace impl
	{
		template <typename T>
		static void _PrintMatrix(string& s, UInt32 width, UInt32 height, const T& value)
		{
			Float32 maxNum = Float32(value.maxCoeff() > Abs(value.minCoeff()) ? value.maxCoeff() : value.minCoeff());
			eastl::string num = eastl::to_string(maxNum);
			UInt32 maxBit = (UInt32)num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (value.minCoeff() < 0)
				bHasNegative = true;

			for (UInt32 i = 0; i < width; i++)
			{
				s += "| ";
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
				s += '|';
				if (i != width - 1)
					s += '\n';
			}
		}

		template<>
		static eastl::string PrintMathTypes<Matrix3f>(const Matrix3f& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Matrix3f:\n";

			_PrintMatrix(s, 3, 3, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix4f>(const Matrix4f& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Matrix4f:\n";

			_PrintMatrix(s, 4, 4, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix3i>(const Matrix3i& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Matrix3i:\n";

			_PrintMatrix(s, 3, 3, types);
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix4i>(const Matrix4i& types, const string& prefix)
		{
			eastl::string s = "";
			if (!prefix.empty())
				s = prefix;
			s += "Matrix4i:\n";

			_PrintMatrix(s, 4, 4, types);
			return eastl::move(s);
		}
	}
}