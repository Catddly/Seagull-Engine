#pragma once

#include "Common/Base/BasicTypes.h"

#include "Common/System/ILog.h"
#include "Common/Memory/IMemory.h" // overload global new and delete

#include "Common/Stl/string.h"

#include <eigen/Dense>

namespace SG
{

	typedef Eigen::Matrix<Float32, 2, 1> Vector2f;
	typedef Eigen::Matrix<Float32, 3, 1> Vector3f;
	typedef Eigen::Matrix<Float32, 4, 1> Vector4f;

	typedef Eigen::Matrix<Int32, 2, 1>   Vector2i;
	typedef Eigen::Matrix<Int32, 3, 1>   Vector3i;
	typedef Eigen::Matrix<Int32, 4, 1>   Vector4i;

	namespace impl
	{
		template<>
		static eastl::string PrintMathTypes<Vector2f>(const Vector2f& types)
		{
			eastl::string s = "Vector2f:\n";
			Float32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			eastl::string num = eastl::to_string(maxNum);
			Size maxBit = num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 2; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 1)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Vector3f>(const Vector3f& types)
		{
			eastl::string s = "Vector3f:\n";
			Float32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			eastl::string num = eastl::to_string(maxNum);
			Size maxBit = num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 3; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 2)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Vector4f>(const Vector4f& types)
		{
			eastl::string s = "Vector4f:\n";
			Float32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			eastl::string num = eastl::to_string(maxNum);
			Size maxBit = num.substr(0, num.find_first_of('.') + 2).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 4; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 3)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Vector2i>(const Vector2i& types)
		{
			eastl::string s = "Vector2i:\n";
			Int32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			Size maxBit = eastl::to_string(maxNum).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 2; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 1)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Vector3i>(const Vector3i& types)
		{
			eastl::string s = "Vector3i:\n";
			Int32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			Size maxBit = eastl::to_string(maxNum).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 3; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 2)
					s += '\n';
			}
			return eastl::move(s);
		}

		template<>
		static eastl::string PrintMathTypes<Vector4i>(const Vector4i& types)
		{
			eastl::string s = "Vector4i:\n";
			Int32 maxNum = types.maxCoeff() > abs(types.minCoeff()) ? types.maxCoeff() : types.minCoeff();
			Size maxBit = eastl::to_string(maxNum).size();

			bool bHasNegative = false;
			if (types.minCoeff() < 0)
				bHasNegative = true;

			for (int i = 0; i < 4; i++)
			{
				s += "[ ";
				for (int j = 0; j < 1; j++)
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
				s += ']';
				if (i != 3)
					s += '\n';
			}
			return eastl::move(s);
		}
	}

}