#include "StdAfx.h"
#include "RenderResourceRegistry.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Memory/Memory.h"
#include "Render/SwapChain.h"
#include "Scene/Mesh/Mesh.h"
#include "Scene/Mesh/MeshDataArchive.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanCommand.h"

#include "ktx/ktx.h"

namespace SG
{

	void VulkanResourceRegistry::CreateInnerResource()
	{
		// create one big vertex buffer
		BufferCreateDesc vbCI = {};
		vbCI.name = "packed_vertex_buffer";
		vbCI.totalSizeInByte = SG_MAX_PACKED_VERTEX_BUFFER_SIZE;
		vbCI.type = EBufferType::efVertex | EBufferType::efTransfer_Dst;
		mPackedVertexBuffer = VulkanBuffer::Create(mpContext->device, vbCI, true);

		// create one big index buffer
		BufferCreateDesc ibCI = {};
		ibCI.name = "packed_index_buffer";
		ibCI.totalSizeInByte = SG_MAX_PACKED_INDEX_BUFFER_SIZE;
		ibCI.type = EBufferType::efIndex | EBufferType::efTransfer_Dst;
		mPackedIndexBuffer = VulkanBuffer::Create(mpContext->device, ibCI, true);

		BufferCreateDesc ssboCI = {};
		ssboCI.name = "perObjectBuffer";
		ssboCI.totalSizeInByte = sizeof(ObjcetRenderData) * SG_MAX_NUM_OBJECT;
		ssboCI.type = EBufferType::efStorage;
		CreateBuffer(ssboCI);
	}

	void VulkanResourceRegistry::DestroyInnerResource()
	{
		Memory::Delete(mPackedVertexBuffer);
		Memory::Delete(mPackedIndexBuffer);
	}

	void VulkanResourceRegistry::Initialize(const VulkanContext* pContext)
	{
		mpContext = const_cast<VulkanContext*>(pContext);

		CreateInnerResource();

		const Mesh* pSkybox = SSystem()->GetMainScene()->GetSkybox();
		auto& skyboxVertices = MeshDataArchive::GetInstance()->GetData(pSkybox->GetMeshID())->vertices;
		const UInt64 vbSize = skyboxVertices.size() * sizeof(float);

		BufferCreateDesc bufferCI = {};
		bufferCI.name = "skybox_vb";
		bufferCI.pInitData = skyboxVertices.data();
		bufferCI.totalSizeInByte = static_cast<UInt32>(vbSize);
		bufferCI.type = EBufferType::efVertex;
		CreateBuffer(bufferCI, true);
		FlushBuffers();

		// eliminate the translation part of the matrix
		ObjcetRenderData renderData = { Matrix4f(Matrix3f(SSystem()->GetMainScene()->GetMainCamera()->GetViewMatrix())) };
		DrawCall renderMesh = {};
		mSkyboxRenderMesh.vBSize = vbSize;
		mSkyboxRenderMesh.iBSize = 0;
		mSkyboxRenderMesh.vBOffset = 0;
		mSkyboxRenderMesh.iBOffset = 0;
		mSkyboxRenderMesh.objectId = 0;
		mSkyboxRenderMesh.instanceCount = 1;
		mSkyboxRenderMesh.pVertexBuffer = GetBuffer("skybox_vb");
		mSkyboxRenderMesh.pIndexBuffer = nullptr;
		mSkyboxRenderMesh.pInstanceBuffer = nullptr;
	}

	void VulkanResourceRegistry::Shutdown()
	{
		DestroyInnerResource();

		// release all the memory
		for (auto beg = mBuffers.begin(); beg != mBuffers.end(); ++beg)
			Memory::Delete(beg->second);
		for (auto beg = mTextures.begin(); beg != mTextures.end(); ++beg)
			Memory::Delete(beg->second);
		for (auto beg = mRenderTargets.begin(); beg != mRenderTargets.end(); ++beg)
			Memory::Delete(beg->second);
		for (auto beg = mSamplers.begin(); beg != mSamplers.end(); ++beg)
			Memory::Delete(beg->second);
	}

	void VulkanResourceRegistry::OnUpdate(WeakRefPtr<Scene> pScene)
	{
		auto pLScene = pScene.lock();
		auto* pSSBOObject = GetBuffer("perObjectBuffer");

		// update all the render data of the render mesh
		pLScene->TraverseMesh([&](const Mesh& mesh)
			{
				if (mStaticRenderMeshes.count(mesh.GetMeshID()) != 0)
				{
					ObjcetRenderData renderData;
					renderData.model = mesh.GetTransform();
					renderData.inverseTransposeModel = glm::transpose(glm::inverse(renderData.model));
					pSSBOObject->UploadData(&renderData, sizeof(ObjcetRenderData), sizeof(ObjcetRenderData) * mesh.GetObjectID());
				}

				if (mStaticRenderMeshesInstanced.count(mesh.GetMeshID()) != 0)
				{
					auto* pInstanceBuffer = GetBuffer((string("instance_vb_") + eastl::to_string(mesh.GetMeshID())));
					if (pInstanceBuffer)
					{
						PerInstanceData data = {};
						data.instancePos = mesh.GetPosition();
						data.instanceScale = mesh.GetScale().x;
						pInstanceBuffer->UploadData(&data, sizeof(PerInstanceData), sizeof(PerInstanceData) * mesh.GetInstanceID());
					}
				}
			});
	}

	VulkanResourceRegistry* VulkanResourceRegistry::GetInstance()
	{
		static VulkanResourceRegistry instance;
		return &instance;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// RenderMesh
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	void VulkanResourceRegistry::BuildRenderMesh(RefPtr<RenderDataBuilder> renderDataBuilder)
	{
		renderDataBuilder->TraverseRenderData([&](UInt32 meshId, const RenderMeshBuildData& buildData)
			{
				if (buildData.instanceCount > 1) // move it to the Forward Instance Mesh Pass
				{
					BufferCreateDesc bufferCreateDesc = {};
					bufferCreateDesc.name = (string("instance_vb_") + eastl::to_string(meshId)).c_str();
					bufferCreateDesc.pInitData = buildData.perInstanceData.data();
					bufferCreateDesc.totalSizeInByte = sizeof(float) * 4 * buildData.instanceCount;
					bufferCreateDesc.type = EBufferType::efVertex;
					CreateBuffer(bufferCreateDesc, false);

					SG_LOG_DEBUG("Have instance!");
				}

				auto* pMeshData = MeshDataArchive::GetInstance()->GetData(meshId);
				const UInt64 vbSize = pMeshData->vertices.size() * sizeof(float);
				const UInt64 ibSize = pMeshData->indices.size() * sizeof(UInt32);

				BufferCreateDesc stagingBufferCI = {};
				stagingBufferCI.totalSizeInByte = static_cast<UInt32>(vbSize);
				stagingBufferCI.type = EBufferType::efTransfer_Src;
				auto* pVBStagingBuffer = VulkanBuffer::Create(mpContext->device, stagingBufferCI, false);
				pVBStagingBuffer->UploadData(pMeshData->vertices.data());

				if (mPackedVBCurrOffset + vbSize > SG_MAX_PACKED_VERTEX_BUFFER_SIZE)
				{
					SG_LOG_ERROR("Vertex buffer exceed boundary!");
					SG_ASSERT(false);
				}

				VulkanBuffer* pIBStagingBuffer = nullptr;
				if (ibSize != 0)
				{
					stagingBufferCI.totalSizeInByte = static_cast<UInt32>(ibSize);;
					pIBStagingBuffer = VulkanBuffer::Create(mpContext->device, stagingBufferCI, false);
					pIBStagingBuffer->UploadData(pMeshData->indices.data());

					if (mPackedIBCurrOffset + ibSize > SG_MAX_PACKED_INDEX_BUFFER_SIZE)
					{
						SG_LOG_ERROR("Index buffer exceed boundary!");
						SG_ASSERT(false);
					}
				}

				VulkanCommandBuffer pCmd;
				mpContext->transferCommandPool->AllocateCommandBuffer(pCmd);

				pCmd.BeginRecord();
				pCmd.CopyBuffer(*pVBStagingBuffer, *mPackedVertexBuffer, 0, mPackedVBCurrOffset);
				if (pIBStagingBuffer)
					pCmd.CopyBuffer(*pIBStagingBuffer, *mPackedIndexBuffer, 0, mPackedIBCurrOffset);
				pCmd.EndRecord();

				mpContext->transferQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
				mpContext->transferQueue.WaitIdle();

				mpContext->transferCommandPool->FreeCommandBuffer(pCmd);

				DrawCall renderMesh = {};
				renderMesh.vBSize = vbSize;
				renderMesh.iBSize = ibSize;
				renderMesh.vBOffset = mPackedVBCurrOffset;
				renderMesh.iBOffset = mPackedIBCurrOffset;
				renderMesh.objectId = buildData.objectId;
				renderMesh.instanceCount = buildData.instanceCount;
				renderMesh.pVertexBuffer = mPackedVertexBuffer;
				renderMesh.pIndexBuffer = mPackedIndexBuffer;
				renderMesh.pInstanceBuffer = nullptr;

				if (buildData.instanceCount > 1)
				{
					renderMesh.pInstanceBuffer = GetBuffer((string("instance_vb_") + eastl::to_string(meshId)));
					mStaticRenderMeshesInstanced[meshId] = renderMesh;
				}
				else
					mStaticRenderMeshes[meshId] = renderMesh;

				mPackedVBCurrOffset += vbSize;
				mPackedIBCurrOffset += ibSize;

				Memory::Delete(pVBStagingBuffer);
				if (pIBStagingBuffer)
					Memory::Delete(pIBStagingBuffer);
			});
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Buffers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal)
	{
		if (mBuffers.count(bufferCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a render buffer named %s!", bufferCI.name);
			return false;
		}

		auto& bufferCreateInfo = const_cast<BufferCreateDesc&>(bufferCI);
		if (bLocal && !SG_HAS_ENUM_FLAG(bufferCreateInfo.type, EBufferType::efTransfer_Dst))
			bufferCreateInfo.type = bufferCreateInfo.type | EBufferType::efTransfer_Dst;
		mBuffers[bufferCreateInfo.name] = VulkanBuffer::Create(mpContext->device, bufferCreateInfo, bLocal);
		VulkanBuffer* pBuffer = mBuffers[bufferCI.name];
		if (!pBuffer)
		{
			SG_LOG_ERROR("Failed to create buffer: %s", bufferCI.name);
			return false;
		}

		if (!bLocal && bufferCI.pInitData)
			pBuffer->UploadData(bufferCI.pInitData);

		if (bLocal)
		{
			if (!bufferCI.pInitData)
			{
				SG_LOG_ERROR("Device local buffer must have initialize data!");
				return false;
			}
			// make a copy of bufferCI
			mWaitToSubmitBuffers.push_back({ bufferCI, pBuffer });
		}
		return true;
	}

	void VulkanResourceRegistry::FlushBuffers() const
	{
		if (mWaitToSubmitBuffers.empty())
			return;

		VulkanCommandBuffer pCmd;
		mpContext->transferCommandPool->AllocateCommandBuffer(pCmd);

		pCmd.BeginRecord();
		vector<VulkanBuffer*> stagingBuffers;
		UInt32 index = 0;
		for (auto& data : mWaitToSubmitBuffers) // copy all the buffers data using staging buffer
		{
			data.first.type = EBufferType::efTransfer_Src;
			stagingBuffers.emplace_back(VulkanBuffer::Create(mpContext->device, data.first, false));
			stagingBuffers[index]->UploadData(data.first.pInitData);
			pCmd.CopyBuffer(*stagingBuffers[index], *data.second);
			++index;
		}
		pCmd.EndRecord();

		mpContext->transferQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mpContext->transferQueue.WaitIdle();

		mpContext->transferCommandPool->FreeCommandBuffer(pCmd);
		for (auto* e : stagingBuffers)
			Memory::Delete(e);
		mWaitToSubmitBuffers.clear();
	}

	VulkanBuffer* VulkanResourceRegistry::GetBuffer(const string& name) const
	{
		if (mBuffers.count(name) == 0)
			return nullptr;
		return mBuffers[name];
	}

	void VulkanResourceRegistry::DeleteBuffer(const string& name)
	{
		auto* pBuffer = GetBuffer(name);
		if (pBuffer)
		{
			Memory::Delete(pBuffer);
			mBuffers.erase(name);
		}
	}

	bool VulkanResourceRegistry::HaveBuffer(const char* name)
	{
		if (mBuffers.count(name) == 0)
			return false;
		return true;
	}

	bool VulkanResourceRegistry::UpdataBufferData(const char* name, const void* pData)
	{
		if (mBuffers.count(name) == 0)
		{
			SG_LOG_ERROR("No buffer named: %s", name);
			return false;
		}
		return mBuffers[name]->UploadData(pData);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Textures
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateTexture(const TextureCreateDesc& textureCI, bool bLocal)
	{
		if (mTextures.count(textureCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a texture named %s!", textureCI.name);
			return false;
		}

		auto& texCI = const_cast<TextureCreateDesc&>(textureCI);
		if (bLocal && !SG_HAS_ENUM_FLAG(textureCI.usage, EImageUsage::efTransfer_Dst))
			texCI.usage = texCI.usage | EImageUsage::efTransfer_Dst;
		mTextures[texCI.name] = VulkanTexture::Create(mpContext->device, textureCI, bLocal);
		VulkanTexture* pTex = mTextures[texCI.name];
		if (!pTex)
		{
			SG_LOG_ERROR("Failed to create texture: %s", texCI.name);
			return false;
		}

		if (bLocal && textureCI.pInitData)
		{
			//if (!textureCI.pInitData)
			//{
			//	SG_LOG_ERROR("Device local buffer must have initialize data!");
			//	return false;
			//}
			// make a copy of bufferCI
			BufferCreateDesc bufferCI = {};
			bufferCI.name = textureCI.name;
			bufferCI.bLocal = bLocal;
			bufferCI.type = EBufferType::efTransfer_Src;
			bufferCI.pInitData = textureCI.pInitData;
			bufferCI.totalSizeInByte = textureCI.sizeInByte;
			mWaitToSubmitTextures.push_back({ bufferCI, pTex });
		}
		return true;
	}

	VulkanTexture* VulkanResourceRegistry::GetTexture(const string& name) const
	{
		if (mTextures.count(name) == 0)
		{
			//SG_LOG_ERROR("No texture named: %s", name);
			return nullptr;
		}
		return mTextures[name];
	}

	void VulkanResourceRegistry::FlushTextures() const
	{
		if (mWaitToSubmitTextures.empty())
			return;

		VulkanCommandBuffer pCmd;
		mpContext->graphicCommandPool->AllocateCommandBuffer(pCmd);

		pCmd.BeginRecord();
		vector<VulkanBuffer*> stagingBuffers;
		UInt32 index = 0;
		for (auto& data : mWaitToSubmitTextures) // copy all the buffers data using staging buffer
		{
			data.first.type = EBufferType::efTransfer_Src;
			stagingBuffers.emplace_back(VulkanBuffer::Create(mpContext->device, data.first, false)); // create a staging buffer
			stagingBuffers[index]->UploadData(data.first.pInitData);

			vector<TextureCopyRegion> copyRegions;
			UInt32 offset = 0;
			for (UInt32 face = 0; face < data.second->GetNumArray(); ++face)
			{
				for (UInt32 i = 0; i < data.second->GetNumMipmap(); ++i)
				{
					TextureCopyRegion copyRegion = {};
					copyRegion.mipLevel = i;
					copyRegion.baseArray = face;
					copyRegion.layer  = 1;
					copyRegion.width  = data.second->GetWidth() >> i;
					copyRegion.height = data.second->GetHeight() >> i;
					copyRegion.depth  = 1;
					if (data.second->GetNumArray() == 6) // cubemap
					{
						Size offset;
						auto ret = ktxTexture_GetImageOffset(reinterpret_cast<ktxTexture*>(data.second->GetUserData()), i, 0, face, &offset);
						copyRegion.offset = static_cast<UInt32>(offset);
					}
					else
					{
						copyRegion.offset = offset;
						offset += copyRegion.width * copyRegion.height * ImageFormatToMemoryByte(data.second->GetFormat()); // 4 byte for RGBA
					}
					copyRegions.emplace_back(copyRegion);
				}
			}
			// transfer image barrier to do the copy
			pCmd.ImageBarrier(data.second, EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
			pCmd.CopyBufferToImage(*stagingBuffers[index], *data.second, copyRegions);
			// transfer complete, switch this texture to a shader read-only texture
			pCmd.ImageBarrier(data.second, EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
			++index;

			if (data.second->GetNumArray() == 6) // cubemap resource destroy
				ktxTexture_Destroy(reinterpret_cast<ktxTexture*>(data.second->GetUserData()));
		}
		pCmd.EndRecord();

		mpContext->graphicQueue.SubmitCommands(&pCmd, nullptr, nullptr, nullptr);
		mpContext->graphicQueue.WaitIdle();

		mpContext->graphicCommandPool->FreeCommandBuffer(pCmd);
		for (auto* e : stagingBuffers)
			Memory::Delete(e);
		mWaitToSubmitTextures.clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// RenderTargets	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateRenderTarget(const TextureCreateDesc& textureCI, bool isDepth)
	{
		if (mRenderTargets.count(textureCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a render target named %s!", textureCI.name);
			return false;
		}

		mRenderTargets[textureCI.name] = VulkanRenderTarget::Create(mpContext->device, textureCI, isDepth);
		VulkanRenderTarget* pRt = mRenderTargets[textureCI.name];
		if (!pRt)
		{
			SG_LOG_ERROR("Failed to create render target : %s", textureCI.name);
			return false;
		}

		return true;
	}

	void VulkanResourceRegistry::DeleteRenderTarget(const string& name)
	{
		auto* pRt = GetRenderTarget(name);
		if (pRt)
		{
			Memory::Delete(pRt);
			mRenderTargets.erase(name);
		}
	}

	VulkanRenderTarget* VulkanResourceRegistry::GetRenderTarget(const string& name) const
	{
		if (mRenderTargets.count(name) == 0)
		{
			//SG_LOG_ERROR("No render target named: %s", name);
			return nullptr;
		}
		return mRenderTargets[name];
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Samplers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateSampler(const SamplerCreateDesc& samplerCI)
	{
		if (mSamplers.count(samplerCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a sampler named %s!", samplerCI.name);
			return false;
		}

		mSamplers[samplerCI.name] = VulkanSampler::Create(mpContext->device, samplerCI);
		VulkanSampler* pTex = mSamplers[samplerCI.name];
		if (!pTex)
		{
			SG_LOG_ERROR("Failed to create sampler: %s", samplerCI.name);
			return false;
		}
		return true;
	}

	VulkanSampler* VulkanResourceRegistry::GetSampler(const string& name) const
	{
		if (mSamplers.count(name) == 0)
		{
			SG_LOG_ERROR("No sampler named: %s", name);
			return nullptr;
		}
		return mSamplers[name];
	}

}