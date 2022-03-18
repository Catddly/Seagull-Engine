#include "StdAfx.h"
#include "VulkanGeometry.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "Stl/string.h"

namespace SG
{

	VulkanGeometry::VulkanGeometry(VulkanContext& d, const string& name, const float* pVerticies, const UInt32 numVertex, const UInt32* pIndices, const UInt32 numIndex)
		: mContext(d), mName(name), mpVertexBuffer(nullptr), mpIndexBuffer(nullptr)
	{
		auto vbBufferCI = InitVertexBuffer(pVerticies, numVertex);

		BufferCreateDesc ibBufferCI = {};
		ibBufferCI.totalSizeInByte = sizeof(UInt32) * numIndex;
		if (numIndex != 0)
		{
			ibBufferCI.name = (name + "_ib").c_str();
			ibBufferCI.pInitData = pIndices;
			ibBufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;

			mpIndexBuffer = VulkanBuffer::Create(mContext.device, ibBufferCI, true);
			if (!mpIndexBuffer)
			{
				SG_LOG_ERROR("Failed to create index buffer of geometry %s", name.c_str());
				SG_ASSERT(false);
			}
		}

		FlushVBIBStagingBuffer(vbBufferCI, ibBufferCI);
	}

	VulkanGeometry::VulkanGeometry(VulkanContext& d, const string& name, const float* pVerticies, const UInt32 numVertex, const UInt16* pIndices, const UInt32 numIndex)
		: mContext(d), mName(name)
	{
		auto vbBufferCI = InitVertexBuffer(pVerticies, numVertex);

		BufferCreateDesc ibBufferCI = {};
		ibBufferCI.name = (name + "_ib").c_str();
		ibBufferCI.pInitData = pIndices;
		ibBufferCI.totalSizeInByte = sizeof(UInt16) * numIndex;
		ibBufferCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;

		mpIndexBuffer = VulkanBuffer::Create(mContext.device, ibBufferCI, true);
		if (!mpIndexBuffer)
		{
			SG_LOG_ERROR("Failed to create index buffer of geometry %s", name.c_str());
			SG_ASSERT(false);
		}

		FlushVBIBStagingBuffer(vbBufferCI, ibBufferCI);
	}

	VulkanGeometry::~VulkanGeometry()
	{
		Memory::Delete(mpVertexBuffer);
		if (mpIndexBuffer)
			Memory::Delete(mpIndexBuffer);
	}

	BufferCreateDesc VulkanGeometry::InitVertexBuffer(const float* pVerticies, UInt32 numVertex)
	{
		BufferCreateDesc vbBufferCI = {};
		vbBufferCI.name = (mName + "_vb").c_str();
		vbBufferCI.pInitData = pVerticies;
		vbBufferCI.totalSizeInByte = sizeof(float) * numVertex;
		vbBufferCI.type = EBufferType::efVertex | EBufferType::efTransfer_Dst;

		mpVertexBuffer = VulkanBuffer::Create(mContext.device, vbBufferCI, true);
		if (!mpVertexBuffer)
		{
			SG_LOG_ERROR("Failed to create vertex buffer of geometry %s", mName.c_str());
			SG_ASSERT(false);
		}
		return vbBufferCI;
	}

	void VulkanGeometry::FlushVBIBStagingBuffer(BufferCreateDesc& vbCI, BufferCreateDesc& ibCI)
	{
		vbCI.type = EBufferType::efTransfer_Src;
		auto* mpVBStagingBuffer = VulkanBuffer::Create(mContext.device, vbCI, false);

		VulkanBuffer* mpIBStagingBuffer = nullptr;
		if (ibCI.totalSizeInByte != 0)
		{
			ibCI.type = EBufferType::efTransfer_Src;
			mpIBStagingBuffer = VulkanBuffer::Create(mContext.device, ibCI, false);
		}

		VulkanCommandBuffer pCmd;
		mContext.transferCommandPool->AllocateCommandBuffer(pCmd);

		pCmd.BeginRecord();
			mpVBStagingBuffer->UploadData(vbCI.pInitData);
			pCmd.CopyBuffer(*mpVBStagingBuffer, *mpVertexBuffer);

			if (ibCI.totalSizeInByte != 0)
			{
				mpIBStagingBuffer->UploadData(ibCI.pInitData);
				pCmd.CopyBuffer(*mpIBStagingBuffer, *mpIndexBuffer);
			}
		pCmd.EndRecord();

		mContext.transferQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mContext.transferQueue.WaitIdle();

		mContext.transferCommandPool->FreeCommandBuffer(pCmd);

		Memory::Delete(mpVBStagingBuffer);
		if (ibCI.totalSizeInByte != 0)
			Memory::Delete(mpIBStagingBuffer);
	}

}