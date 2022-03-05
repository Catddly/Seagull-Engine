#pragma once

#include "Archive/ResourceLoader/ResourceDefs.h"

#include "Stl/vector.h"

namespace SG
{

	class Mesh
	{
	public:
		explicit Mesh(const char* name, EMeshType type);
		~Mesh() = default;

		const char* GetName() const noexcept { return mName; }
		const EMeshType GetType() const noexcept { return mType; }

		const vector<float>&  GetVertices() const noexcept { return mVertices; }
		const vector<UInt32>& GetIndices()  const noexcept { return mIndices; }
	private:
		const char*    mName;
		EMeshType      mType;
		vector<float>  mVertices;
		vector<UInt32> mIndices;
	};

}