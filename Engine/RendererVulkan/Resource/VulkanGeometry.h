#pragma once

#include "Render/Buffer.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	class VulkanContext;
	class VulkanBuffer;

	class VulkanGeometry
	{
	public:
		VulkanGeometry(VulkanContext& d, const string& name, const float* pVerticies, const UInt32 numVertex, const UInt32* pIndices, const UInt32 numIndex);
		VulkanGeometry(VulkanContext& d, const string& name, const float* pVerticies, const UInt32 numVertex, const UInt16* pIndices, const UInt32 numIndex);

		~VulkanGeometry();

		SG_INLINE VulkanBuffer* GetVertexBuffer() const { return mpVertexBuffer; }
		SG_INLINE VulkanBuffer* GetIndexBuffer()  const { return mpIndexBuffer; }
	private:
		BufferCreateDesc InitVertexBuffer(const float* pVerticies, UInt32 numVertex);
		void FlushVBIBStagingBuffer(BufferCreateDesc& vbCI, BufferCreateDesc& ibCI);
	private:
		VulkanContext& mContext;

		string        mName;
		VulkanBuffer* mpVertexBuffer;
		VulkanBuffer* mpIndexBuffer;
	};

}