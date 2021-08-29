#pragma once

#include "Base/BasicTypes.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h" // overload global new and delete

#include <eigen/Dense>

namespace SG
{
	typedef Eigen::Matrix<Float32, 3, 3> Matrix3f;
	typedef Eigen::Matrix<Float32, 4, 4> Matrix4f;

	typedef Eigen::Matrix<Int32, 3, 3>   Matrix3i;
	typedef Eigen::Matrix<Int32, 4, 4>   Matrix4i;

	// overload log out function
	namespace impl
	{
		template<>
		static eastl::string PrintMathTypes<Matrix3f>(const Matrix3f& types)
		{
			eastl::string s = "Matrix3f:\n";
			Float32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			eastl::string num = eastl::to_string(maxNum);
			Size maxBit = num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 3; i++)
			{
				s += "| ";
				for (int j = 0; j < 3; j++)
				{
					Size tempBit = maxBit + 1;
					if (types(i, j) > 0 && bHasNegative)
					{
						s += ' ';
						tempBit -= 1;
					}
					eastl::string num = eastl::to_string(types(i, j));
					eastl::string clipedNum = num.substr(0, num.find_first_of('.') + 2);
					tempBit -= clipedNum.size();
					s += clipedNum;
					for (int k = 0; k < tempBit; k++)
						s += ' ';
				}
				s += '|';
				if (i != 2)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix4f>(const Matrix4f& types)
		{
			eastl::string s = "Matrix4f:\n";
			Float32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			eastl::string num = eastl::to_string(maxNum);
			Size maxBit = num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 4; i++)
			{
				s += "| ";
				for (int j = 0; j < 4; j++)
				{
					Size tempBit = maxBit + 1;
					if (types(i, j) > 0 && bHasNegative)
					{
						s += ' ';
						tempBit -= 1;
					}
					eastl::string num = eastl::to_string(types(i, j));
					eastl::string clipedNum = num.substr(0, num.find_first_of('.') + 2);
					tempBit -= clipedNum.size();
					s += clipedNum;
					for (int k = 0; k < tempBit; k++)
						s += ' ';
				}
				s += '|';
				if (i != 3)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix3i>(const Matrix3i& types)
		{
			eastl::string s = "Matrix3i:\n";
			Int32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			Size maxBit = eastl::to_string(maxNum).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 3; i++)
			{
				s += "| ";
				for (int j = 0; j < 3; j++)
				{
					Size tempBit = maxBit + 1;
					if (types(i, j) > 0 && bHasNegative)
					{
						s += ' ';
						tempBit -= 1;
					}
					eastl::string num = eastl::to_string(types(i, j));
					tempBit -= num.size();
					s += num;
					for (int k = 0; k < tempBit; k++)
						s += ' ';
				}
				s += '|';
				if (i != 2)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Matrix4i>(const Matrix4i& types)
		{
			eastl::string s = "Matrix4i:\n";
			Int32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			Size maxBit = eastl::to_string(maxNum).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)

				bHasNegative = true;
			for (int i = 0; i < 4; i++)
			{
				s += "| ";
				for (int j = 0; j < 4; j++)
				{
					Size tempBit = maxBit + 1;
					if (types(i, j) > 0 && bHasNegative)
					{
						s += ' ';
						tempBit -= 1;
					}
					eastl::string num = eastl::to_string(types(i, j));
					tempBit -= num.size();
					s += num;
					for (int k = 0; k < tempBit; k++)
						s += ' ';
				}
				s += '|';
				if (i != 3)
					s += '\n';
			}
			return eastl::move(s);
		}
	}
}