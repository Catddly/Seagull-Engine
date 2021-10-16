#pragma once

#include "Base/BasicTypes.h"

#include "System/ILogger.h"
#include "Memory/IMemory.h" // overload global new and delete

#include "Stl/string.h"

#include <eigen/Dense>

namespace SG
{
	typedef Eigen::Vector2f Vector2f;
	typedef Eigen::Vector3f Vector3f;
	typedef Eigen::Vector4f Vector4f;

	typedef Eigen::Vector2i Vector2i;
	typedef Eigen::Vector3i Vector3i;
	typedef Eigen::Vector4i Vector4i;

	// overload log out functions
	namespace impl
	{
		template<>
		static eastl::string PrintMathTypes<SG::Vector2f>(const SG::Vector2f& types)
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
		static eastl::string PrintMathTypes<SG::Vector3f>(const SG::Vector3f& types)
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
		static eastl::string PrintMathTypes<SG::Vector4f>(const SG::Vector4f& types)
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
		static eastl::string PrintMathTypes<SG::Vector2i>(const SG::Vector2i& types)
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
		static eastl::string PrintMathTypes<SG::Vector3i>(const SG::Vector3i& types)
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
		static eastl::string PrintMathTypes<SG::Vector4i>(const SG::Vector4i& types)
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