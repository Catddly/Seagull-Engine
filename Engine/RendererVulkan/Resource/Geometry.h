#pragma once

#include "Render/Buffer.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	class VulkanContext;
	class VulkanBuffer;

	class Geometry
	{
	public:
		Geometry(VulkanContext& d, const string& name, float* pVerticies, UInt32 numVertex, UInt32* pIndices, UInt32 numIndex);
		Geometry(VulkanContext& d, const string& name, float* pVerticies, UInt32 numVertex, UInt16* pIndices, UInt16 numIndex);

		VulkanBuffer* GetVertexBuffer() const;
		VulkanBuffer* GetIndexBuffer() const;

		~Geometry();
	private:
		BufferCreateDesc InitVertexBuffer(float* pVerticies, UInt32 numVertex);
		void FlushVBIBStagingBuffer(BufferCreateDesc& vbCI, BufferCreateDesc& ibCI);
	private:
		VulkanContext& mContext;

		string        mName;
		VulkanBuffer* mpVertexBuffer;
		VulkanBuffer* mpIndexBuffer;
	};

}