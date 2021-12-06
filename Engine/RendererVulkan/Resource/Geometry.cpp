#include "StdAfx.h"
#include "Geometry.h"

#include "RendererVulkan/Resource/RenderResourceRegistry.h"

#include "Stl/string.h"

namespace SG
{

	Geometry::Geometry(const char* name, float* pVerticies, UInt32 numVertex, UInt32* pIndices, UInt32 numIndex)
		: mName(name)
	{
		string resourceName = name;

		// vertex buffer
		BufferCreateDesc BufferCI = {};
		BufferCI.name = (resourceName + string("_vb")).c_str();
		BufferCI.totalSizeInByte = sizeof(float) * numVertex;
		BufferCI.type = EBufferType::efVertex;
		BufferCI.pInitData = pVerticies;
		VK_RESOURCE()->CreateBuffer(BufferCI, true);

		// index buffer
		BufferCI.name = (resourceName + string("_ib")).c_str();
		BufferCI.totalSizeInByte = sizeof(UInt32) * numIndex;
		BufferCI.type = EBufferType::efIndex;
		BufferCI.pInitData = pIndices;
		VK_RESOURCE()->CreateBuffer(BufferCI, true);
	}

	Geometry::Geometry(const char* name, float* pVerticies, UInt32 numVertex, UInt16* pIndices, UInt16 numIndex)
		: mName(name)
	{
		string resourceName = name;

		// vertex buffer
		BufferCreateDesc BufferCI = {};
		BufferCI.name = (resourceName + string("_vb")).c_str();
		BufferCI.totalSizeInByte = sizeof(float) * numVertex;
		BufferCI.type = EBufferType::efVertex;
		BufferCI.pInitData = pVerticies;
		VK_RESOURCE()->CreateBuffer(BufferCI, true);

		// index buffer
		BufferCI.name = (resourceName + string("_ib")).c_str();;
		BufferCI.totalSizeInByte = sizeof(UInt16) * numIndex;
		BufferCI.type = EBufferType::efIndex;
		BufferCI.pInitData = pIndices;
		VK_RESOURCE()->CreateBuffer(BufferCI, true);
	}

	VulkanBuffer* Geometry::GetVertexBuffer() const
	{
		string name = mName + string("_vb");
		return VK_RESOURCE()->GetBuffer(name);
	}

	VulkanBuffer* Geometry::GetIndexBuffer() const
	{
		string name = mName + string("_ib");
		return VK_RESOURCE()->GetBuffer(name);
	}

}