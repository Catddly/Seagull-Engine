#include "StdAfx.h"
#include "RenderResourceRegistry.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Memory/Memory.h"
#include "Render/SwapChain.h"
#include "Archive/MeshDataArchive.h"
#include "Profile/Profile.h"
#include "Render/CommonRenderData.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanQueue.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanTexture.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

#include "ktx/ktx.h"

namespace SG
{

	void VulkanResourceRegistry::Initialize(const VulkanContext* pContext)
	{
		SG_PROFILE_FUNCTION();

		mpContext = const_cast<VulkanContext*>(pContext);

		BufferCreateDesc ssboCI = {};
		ssboCI.name = "perObjectBuffer";
		ssboCI.bufferSize = sizeof(ObjcetRenderData) * SG_MAX_NUM_OBJECT;
		ssboCI.type = EBufferType::efStorage;
		ssboCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;
		ssboCI.memoryFlag = EGPUMemoryFlag::efPersistent_Map;
		CreateBuffer(ssboCI);

		// This is only for debugging purpose
		//ssboCI = {};
		//ssboCI.name = "cullingOutputData";
		//ssboCI.bufferSize = sizeof(CullingOutputData) * SG_MAX_NUM_OBJECT;
		//ssboCI.type = EBufferType::efStorage;
		//CreateBuffer(ssboCI);

#if SG_ENABLE_GPU_CULLING
		BufferCreateDesc cullBufferCI = {};
		cullBufferCI.name = "cullUbo";
		cullBufferCI.bufferSize = sizeof(GPUCullUBO) * SG_MAX_NUM_OBJECT;
		cullBufferCI.type = EBufferType::efUniform;
		cullBufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;
		cullBufferCI.memoryFlag = EGPUMemoryFlag::efPersistent_Map;
		CreateBuffer(cullBufferCI);
#endif

		auto pSkybox = SSystem()->GetMainScene()->GetSkyboxEntity();
		auto& mesh = pSkybox.GetComponent<MeshComponent>();
		auto& skyboxVertices = MeshDataArchive::GetInstance()->GetData(mesh.meshId)->vertices;
		const UInt64 vbSize = skyboxVertices.size() * sizeof(float);

		BufferCreateDesc bufferCI = {};
		bufferCI.name = "skybox_vb";
		bufferCI.pInitData = skyboxVertices.data();
		bufferCI.bufferSize = static_cast<UInt32>(vbSize);
		bufferCI.type = EBufferType::efVertex;
		bufferCI.memoryUsage = EGPUMemoryUsage::eGPU_Only;
		CreateBuffer(bufferCI);
		FlushBuffers();

		// eliminate the translation part of the matrix
		auto& cam = SSystem()->GetMainScene()->GetMainCamera();
		auto pCamera = cam.GetComponent<CameraComponent>().pCamera;
		ObjcetRenderData renderData = { Matrix4f(Matrix3f(pCamera->GetViewMatrix())) };
		DrawCall renderMesh = {};
		mSkyboxDrawCall.drawMesh.vBSize = vbSize;
		mSkyboxDrawCall.drawMesh.iBSize = 0;
		mSkyboxDrawCall.drawMesh.vBOffset = 0;
		mSkyboxDrawCall.drawMesh.iBOffset = 0;
		mSkyboxDrawCall.drawMesh.pVertexBuffer = GetBuffer("skybox_vb");
		mSkyboxDrawCall.drawMesh.pIndexBuffer = nullptr;
		mSkyboxDrawCall.drawMesh.pInstanceBuffer = nullptr;
		mSkyboxDrawCall.objectId = 0;
		mSkyboxDrawCall.instanceCount = 1;
		
		// init uniform buffers
		auto pScene = SSystem()->GetMainScene();

		auto& cameraUbo = GetCameraUBO();
		cameraUbo.proj = pCamera->GetProjMatrix();

#if SG_ENABLE_GPU_CULLING
		Frustum cameraFrustum = pCamera->GetFrustum();
		auto& cullUbo = GetGPUCullUBO();
		cullUbo.frustum[0] = cameraFrustum.GetTopPlane();
		cullUbo.frustum[1] = cameraFrustum.GetBottomPlane();
		cullUbo.frustum[2] = cameraFrustum.GetLeftPlane();
		cullUbo.frustum[3] = cameraFrustum.GetRightPlane();
		cullUbo.frustum[4] = cameraFrustum.GetFrontPlane();
		cullUbo.frustum[5] = cameraFrustum.GetBackPlane();
		cullUbo.viewPos = pCamera->GetPosition();
		cullUbo.numObjects = static_cast<UInt32>(pScene->GetMeshEntityCount());
		cullUbo.numDrawCalls = MeshDataArchive::GetInstance()->GetNumMeshData();
		GetBuffer("cullUbo")->UploadData(&cullUbo);
#endif

		auto& skyboxUbo = GetSkyboxUBO();
		skyboxUbo.proj = cameraUbo.proj;

		auto& compositionUbo = GetCompositionUBO();
		compositionUbo.gamma = 2.2f;
		compositionUbo.exposure = 1.0f;

		auto* pSSBOObject = GetBuffer("perObjectBuffer");

		renderData = {};
		renderData.instanceOffset = -1;
		pSSBOObject->UploadData(&renderData, sizeof(ObjcetRenderData), 0);

		// update all the render data of the render mesh
		pScene->TraverseEntityContext([pSSBOObject](Scene::EntityContext& context)
			{
				auto& entity = context.entity;
				if (entity.HasComponent<MeshComponent>() && entity.HasComponent<MaterialComponent>())
				{
					auto [mesh, mat] = entity.GetComponent<MeshComponent, MaterialComponent>();
					auto matAsset = mat.materialAsset.lock();

					ObjcetRenderData renderData = {};
					renderData.model = GetTransform(context.pTreeNode);
					renderData.inverseTransposeModel = glm::transpose(glm::inverse(renderData.model));
					renderData.meshId = mesh.meshId;
					renderData.MR = { matAsset->GetMetallic(), matAsset->GetRoughness() };
					renderData.instanceOffset = MeshDataArchive::GetInstance()->HaveInstance(renderData.meshId) ? MeshDataArchive::GetInstance()->GetInstanceSumOffset(renderData.meshId - 1) + mesh.instanceId : -1;
					renderData.albedo = matAsset->GetAlbedo();
					renderData.texFlag = matAsset->GetTextureMask();
					pSSBOObject->UploadData(&renderData, sizeof(ObjcetRenderData), sizeof(ObjcetRenderData) * mesh.objectId);
				}
			});
	}

	void VulkanResourceRegistry::Shutdown()
	{
		SG_PROFILE_FUNCTION();

		// release all the memory
		for (auto beg = mBuffers.begin(); beg != mBuffers.end(); ++beg)
			Delete(beg->second);
		for (auto beg = mTextures.begin(); beg != mTextures.end(); ++beg)
			Delete(beg->second);
		for (auto beg = mDescriptorSets.begin(); beg != mDescriptorSets.end(); ++beg)
			Delete(beg->second);
		for (auto beg = mRenderTargets.begin(); beg != mRenderTargets.end(); ++beg)
			Delete(beg->second);
		for (auto beg = mSamplers.begin(); beg != mSamplers.end(); ++beg)
			Delete(beg->second);
	}

	void VulkanResourceRegistry::OnUpdate()
	{
		SG_PROFILE_FUNCTION();

		mMessageBusMember.ListenFor("RenderDataRebuild", SG_BIND_MEMBER_FUNC(OnRenderDataRebuild));

		auto pScene = SSystem()->GetMainScene();

		// update camera data
		auto& cam = pScene->GetMainCamera();
		auto pCamera = cam.GetComponent<CameraComponent>().pCamera;
		auto& skyboxUbo = GetSkyboxUBO();
		auto& cameraUbo = GetCameraUBO();

		bool bNeedToUpdateDirectionalLight = false;

		if (pCamera->IsProjDirty())
		{
			cameraUbo.proj = pCamera->GetProjMatrix();
			skyboxUbo.proj = cameraUbo.proj;
		}

		if (pCamera->IsViewDirty())
		{
			cameraUbo.viewPos = pCamera->GetPosition();
			cameraUbo.view = pCamera->GetViewMatrix();
			skyboxUbo.model = Matrix4f(Matrix3f(cameraUbo.view)); // eliminate the translation part of the matrix

			bNeedToUpdateDirectionalLight = true;
		}

		if (pCamera->IsViewDirty() || pCamera->IsProjDirty())
		{
#if SG_ENABLE_GPU_CULLING
			auto& cullUbo = GetGPUCullUBO();
			Frustum cameraFrustum = pCamera->GetFrustum();
			cullUbo.frustum[0] = cameraFrustum.GetTopPlane();
			cullUbo.frustum[1] = cameraFrustum.GetBottomPlane();
			cullUbo.frustum[2] = cameraFrustum.GetLeftPlane();
			cullUbo.frustum[3] = cameraFrustum.GetRightPlane();
			cullUbo.frustum[4] = cameraFrustum.GetFrontPlane();
			cullUbo.frustum[5] = cameraFrustum.GetBackPlane();
			cullUbo.viewPos = pCamera->GetPosition();

			UpdataBufferData("cullUbo", &cullUbo);
#endif

			cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
			UpdataBufferData("cameraUbo", &cameraUbo);
			UpdataBufferData("skyboxUbo", &skyboxUbo);

			pCamera->ProjBeUpdated();
			pCamera->ViewBeUpdated();
		}

		auto* pSSBOObject = GetBuffer("perObjectBuffer");
		bool bNeedUpdateLightUbo = false;
		auto& lightUbo = GetLightUBO();

		pScene->TraverseEntityContext([this, pSSBOObject, &bNeedUpdateLightUbo, &lightUbo, pScene, pCamera, bNeedToUpdateDirectionalLight](Scene::EntityContext& context)
			{
				auto& entity = context.entity;

				auto& tag = entity.GetComponent<TagComponent>();
				if (tag.bDirty)
				{
					if (entity.HasComponent<PointLightComponent>())
					{
						auto [trans, light] = entity.GetComponent<TransformComponent, PointLightComponent>();

						lightUbo.pointLightColor = light.color;
						lightUbo.pointLightRadius = light.radius;
						lightUbo.pointLightPos = trans.position;
						bNeedUpdateLightUbo |= true;
					}
					else if (entity.HasComponent<DirectionalLightComponent>())
					{
						auto [trans, light] = entity.GetComponent<TransformComponent, DirectionalLightComponent>();

						auto& shadowUbo = GetShadowUBO();
						lightUbo.viewDirection = CalcViewDirectionNormalized(trans);
						lightUbo.directionalColor = { light.color, 1.0f };

						float aspect = OperatingSystem::GetMainWindow()->GetAspectRatio();
						shadowUbo.lightSpaceVP = CalcDirectionalLightViewProj(trans, lightUbo.viewDirection, pCamera->GetPosition(), 
							aspect, light.shadowMapScaleFactor, light.zNear, light.zFar);
						lightUbo.lightSpaceVP = shadowUbo.lightSpaceVP;
						UpdataBufferData("shadowUbo", &shadowUbo);
						bNeedUpdateLightUbo |= true;
					}
					else if (entity.HasComponent<MeshComponent>() && entity.HasComponent<MaterialComponent>())
					{
						auto [mesh, mat] = entity.GetComponent<MeshComponent, MaterialComponent>();
						auto matAsset = mat.materialAsset.lock();

						ObjcetRenderData renderData = {};
						renderData.model = GetTransform(context.pTreeNode);
						renderData.inverseTransposeModel = glm::transpose(glm::inverse(renderData.model));
						renderData.meshId = mesh.meshId;
						renderData.MR = { matAsset->GetMetallic(), matAsset->GetRoughness() };
						renderData.instanceOffset = MeshDataArchive::GetInstance()->HaveInstance(renderData.meshId) ? MeshDataArchive::GetInstance()->GetInstanceSumOffset(renderData.meshId - 1) + mesh.instanceId : -1;
						renderData.albedo = matAsset->GetAlbedo();
						renderData.texFlag = matAsset->GetTextureMask();
						pSSBOObject->UploadData(&renderData, sizeof(ObjcetRenderData), sizeof(ObjcetRenderData) * mesh.objectId);
					}
					tag.bDirty = false;
				}
				else if (bNeedToUpdateDirectionalLight)
				{
					if (entity.HasComponent<DirectionalLightComponent>())
					{
						auto [trans, light] = entity.GetComponent<TransformComponent, DirectionalLightComponent>();

						auto& shadowUbo = GetShadowUBO();
						lightUbo.viewDirection = CalcViewDirectionNormalized(trans);
						lightUbo.directionalColor = { light.color, 1.0f };

						float aspect = OperatingSystem::GetMainWindow()->GetAspectRatio();
						shadowUbo.lightSpaceVP = CalcDirectionalLightViewProj(trans, lightUbo.viewDirection, pCamera->GetPosition(), 
							aspect, light.shadowMapScaleFactor, light.zNear, light.zFar);
						lightUbo.lightSpaceVP = shadowUbo.lightSpaceVP;
						UpdataBufferData("shadowUbo", &shadowUbo);
						bNeedUpdateLightUbo |= true;
					}
				}
			});

		if (bNeedUpdateLightUbo)
			UpdataBufferData("lightUbo", &lightUbo);

		auto& compositionUbo = GetCompositionUBO();
		UpdataBufferData("compositionUbo", &compositionUbo);
	}

	void VulkanResourceRegistry::WindowResize()
	{
	}

	VulkanResourceRegistry* VulkanResourceRegistry::GetInstance()
	{
		static VulkanResourceRegistry instance;
		return &instance;
	}

	void VulkanResourceRegistry::OnRenderDataRebuild()
	{
#if SG_ENABLE_GPU_CULLING
		auto pScene = SSystem()->GetMainScene();

		auto& cullUbo = GetGPUCullUBO();
		cullUbo.numObjects = static_cast<UInt32>(pScene->GetMeshEntityCount());
		cullUbo.numDrawCalls = MeshDataArchive::GetInstance()->GetNumMeshData();
		GetBuffer("cullUbo")->UploadData(&cullUbo);
#endif

		// reset lightUbo
		auto& lightUbo = GetLightUBO();
		lightUbo.pointLightColor = Vector3f(0.0f);
		lightUbo.pointLightRadius = 0.0f;
		lightUbo.pointLightPos = Vector3f(0.0f);
		UpdataBufferData("lightUbo", &lightUbo);
	}

	bool VulkanResourceRegistry::IsTextureNeedToGenerateMipmap(UInt32 width, UInt32 height, EImageFormat imageFormat, UInt32 dataByteSize, UInt32 mipmap) const
	{
		return width * height * ImageFormatToMemoryByte(imageFormat) == dataByteSize &&
			mipmap > 1;
	}

	void VulkanResourceRegistry::WaitBuffersUpdated() const
	{
		SG_PROFILE_FUNCTION();

		for (auto* pFence : mpBufferUploadFences)
			pFence->WaitAndReset();

		for (auto& buf : mSubmitedCommandBuffers)
			mpContext->pTransferCommandPool->FreeCommandBuffer(buf);
		for (auto* pStagingBuf : mpStagingBuffers)
			Delete(pStagingBuf);
		for (auto* pFence : mpBufferUploadFences)
			Delete(pFence);

		mpBufferUploadFences.clear();
		mpStagingBuffers.clear();
		mSubmitedCommandBuffers.clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Buffers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateBuffer(const BufferCreateDesc& bufferCI)
	{
		SG_PROFILE_FUNCTION();

		bool bHostVisible = IsHostVisible(bufferCI.memoryUsage);
		auto& bufferCreateInfo = const_cast<BufferCreateDesc&>(bufferCI);
		if (!bHostVisible && !SG_HAS_ENUM_FLAG(bufferCreateInfo.type, EBufferType::efTransfer_Dst))
			bufferCreateInfo.type = bufferCreateInfo.type | EBufferType::efTransfer_Dst;

		if (mBuffers.count(bufferCreateInfo.name) != 0) // do data copy
		{
			if (!bHostVisible)
			{
				if (!bufferCreateInfo.pInitData)
				{
					SG_LOG_ERROR("Device local buffer must have initialize data!");
					return false;
				}
				// make a copy of bufferCI
				mWaitToSubmitBuffers.push_back({ bufferCreateInfo, GetBuffer(bufferCreateInfo.name) });
				return true;
			}
		}

		mBuffers[bufferCreateInfo.name] = VulkanBuffer::Create(*mpContext, bufferCreateInfo);
		VulkanBuffer* pBuffer = mBuffers[bufferCI.name];
		if (!pBuffer)
		{
			SG_LOG_ERROR("Failed to create buffer: %s", bufferCreateInfo.name);
			return false;
		}

		if (bHostVisible && bufferCreateInfo.pInitData)
		{
			if (bufferCreateInfo.bSubBuffer)
				pBuffer->UploadData(bufferCreateInfo.pInitData, bufferCreateInfo.subBufferSize, bufferCreateInfo.subBufferOffset);
			else
				pBuffer->UploadData(bufferCreateInfo.pInitData);
		}

		if (!bHostVisible)
		{
			if (bufferCreateInfo.pInitData)
			{
				// make a copy of bufferCI
				mWaitToSubmitBuffers.push_back({ bufferCreateInfo, pBuffer });
			}
		}
		return true;
	}

	void VulkanResourceRegistry::FlushBuffers() const
	{
		SG_PROFILE_FUNCTION();

		if (mWaitToSubmitBuffers.empty())
			return;

		auto& cmd = mSubmitedCommandBuffers.push_back();
		mpContext->pTransferCommandPool->AllocateCommandBuffer(cmd);
		mpBufferUploadFences.push_back(VulkanFence::Create(mpContext->device));

		cmd.BeginRecord();
		UInt32 index = 0;
		for (auto& data : mWaitToSubmitBuffers) // copy all the buffers data using staging buffer
		{
			data.first.type = EBufferType::efTransfer_Src;
			data.first.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;

			if (data.first.bSubBuffer)
				data.first.bufferSize = data.first.subBufferSize;

			mpStagingBuffers.push_back(VulkanBuffer::Create(*mpContext, data.first));
			auto* pStagingBuf = mpStagingBuffers.back();
			pStagingBuf->UploadData(data.first.pInitData);

			cmd.CopyBuffer(*pStagingBuf, *data.second, 0, data.first.bSubBuffer ? data.first.subBufferOffset : 0);
			++index;
		}
		cmd.EndRecord();

		mpContext->pTransferQueue->SubmitCommands<0, 0, 0>(&cmd, nullptr, nullptr, nullptr, mpBufferUploadFences.back());
		mWaitToSubmitBuffers.clear();
	}

	VulkanBuffer* VulkanResourceRegistry::GetBuffer(const string& name) const
	{
		SG_PROFILE_FUNCTION();

		if (mBuffers.count(name) == 0)
			return nullptr;
		return mBuffers[name];
	}

	void VulkanResourceRegistry::DeleteBuffer(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto* pBuffer = GetBuffer(name);
		if (pBuffer)
		{
			Delete(pBuffer);
			mBuffers.erase(name);
		}
	}

	bool VulkanResourceRegistry::HaveBuffer(const char* name)
	{
		SG_PROFILE_FUNCTION();

		if (mBuffers.find(name) == mBuffers.end())
			return false;
		return true;
	}

	bool VulkanResourceRegistry::UpdataBufferData(const char* name, const void* pData)
	{
		SG_PROFILE_FUNCTION();

		if (mBuffers.count(name) == 0)
		{
			SG_LOG_ERROR("No buffer named: %s", name);
			return false;
		}
		return mBuffers[name]->UploadData(pData);
	}

	bool VulkanResourceRegistry::UpdataBufferData(const char* name, const void* pData, UInt32 size, UInt32 offset)
	{
		SG_PROFILE_FUNCTION();

		if (mBuffers.count(name) == 0)
		{
			SG_LOG_ERROR("No buffer named: %s", name);
			return false;
		}
		return mBuffers[name]->UploadData(pData, size, offset);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Textures
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateTexture(const TextureCreateDesc& textureCI)
	{
		SG_PROFILE_FUNCTION();

		auto& texCI = const_cast<TextureCreateDesc&>(textureCI);
		if (!SG_HAS_ENUM_FLAG(textureCI.usage, EImageUsage::efTransfer_Dst))
			texCI.usage = texCI.usage | EImageUsage::efTransfer_Dst;
		if (IsTextureNeedToGenerateMipmap(textureCI.width, textureCI.height, textureCI.format, textureCI.sizeInByte, textureCI.mipLevel)) // need to generate mipmaps
			texCI.usage = texCI.usage | EImageUsage::efTransfer_Src;

		if (mTextures.count(texCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a texture named %s!", texCI.name);
			return false;
		}

		mTextures[texCI.name] = VulkanTexture::Create(*mpContext, texCI);
		VulkanTexture* pTex = mTextures[texCI.name];
		if (!pTex)
		{
			SG_LOG_ERROR("Failed to create texture: %s", texCI.name);
			return false;
		}

		if (texCI.pInitData)
		{
			//if (!textureCI.pInitData)
			//{
			//	SG_LOG_ERROR("Device local buffer must have initialize data!");
			//	return false;
			//}
			 
			// make a copy of bufferCI
			BufferCreateDesc bufferCI = {};
			bufferCI.name = texCI.name;
			bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;
			bufferCI.type = EBufferType::efTransfer_Src;
			bufferCI.pInitData = texCI.pInitData;
			bufferCI.bufferSize = texCI.sizeInByte;
			mWaitToSubmitTextures.push_back({ bufferCI, pTex });
		}
		return true;
	}

	bool VulkanResourceRegistry::HaveTexture(const char* name) const
	{
		SG_PROFILE_FUNCTION();

		if (mTextures.find(name) == mTextures.end())
			return false;
		return true;
	}

	VulkanTexture* VulkanResourceRegistry::GetTexture(const string& name) const
	{
		SG_PROFILE_FUNCTION();

		if (mTextures.count(name) == 0)
		{
			//SG_LOG_ERROR("No texture named: %s", name);
			return nullptr;
		}
		return mTextures[name];
	}

	void VulkanResourceRegistry::FlushTextures() const
	{
		SG_PROFILE_FUNCTION();

		if (mWaitToSubmitTextures.empty())
			return;

		VulkanCommandBuffer pCmd;
		mpContext->pGraphicCommandPool->AllocateCommandBuffer(pCmd);

		pCmd.BeginRecord();
		vector<VulkanBuffer*> stagingBuffers;
		UInt32 index = 0;
		for (auto& data : mWaitToSubmitTextures) // copy all the buffers data using staging buffer
		{
			auto* pTex = data.second;
			auto& bufferCI = data.first;

			if (IsTextureNeedToGenerateMipmap(pTex->GetWidth(), pTex->GetHeight(), pTex->GetFormat(), bufferCI.bufferSize, pTex->GetNumMipmap())) // need to generate mipmap for this texture
			{
				// create staging buffer
				bufferCI.type = EBufferType::efTransfer_Src;
				bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;

				stagingBuffers.emplace_back(VulkanBuffer::Create(*mpContext, bufferCI)); // create a staging buffer
				stagingBuffers[index]->UploadData(bufferCI.pInitData);

				// copy mipmap 0 data first.
				vector<TextureCopyRegion> copyRegions;
				TextureCopyRegion copyRegion = {};
				copyRegion.mipLevel = 0;
				copyRegion.baseArray = 0;
				copyRegion.layer = 1;
				copyRegion.width = data.second->GetWidth();
				copyRegion.height = data.second->GetHeight();
				copyRegion.depth = 1;
				copyRegion.offset = 0;
				copyRegions.push_back(copyRegion);

				pCmd.ImageBarrier(data.second, EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
				pCmd.CopyBufferToImage(*stagingBuffers[index], *pTex, copyRegions);

				UInt32 mipWidth = copyRegion.width;
				UInt32 mipHeight = copyRegion.height;
				TextureBlitRegion blitRegion = {};
				blitRegion.srcBaseArrayLayer = 0;
				blitRegion.srcLayerCount = 1;
				blitRegion.dstBaseArrayLayer = 0;
				blitRegion.dstLayerCount = 1;
				for (UInt32 i = 1; i < pTex->GetNumMipmap(); ++i) // copy mip-chain from mipmap 1
				{
					pCmd.ImageBarrier(data.second, EResourceBarrier::efCopy_Dest, EResourceBarrier::efCopy_Source, i - 1); // the former mipmap level to copy

					blitRegion.srcOffsets[0][0] = 0;
					blitRegion.srcOffsets[0][1] = 0;
					blitRegion.srcOffsets[0][2] = 0;
					blitRegion.srcOffsets[1][0] = mipWidth;
					blitRegion.srcOffsets[1][1] = mipHeight;
					blitRegion.srcOffsets[1][2] = 1;

					blitRegion.srcMipLevel = i - 1;

					blitRegion.dstOffsets[0][0] = 0;
					blitRegion.dstOffsets[0][1] = 0;
					blitRegion.dstOffsets[0][2] = 0;
					blitRegion.dstOffsets[1][0] = mipWidth > 1 ? mipWidth / 2 : 1;
					blitRegion.dstOffsets[1][1] = mipHeight > 1 ? mipHeight / 2 : 1;
					blitRegion.dstOffsets[1][2] = 1;
					blitRegion.dstMipLevel = i;

					pCmd.BlitImage(*pTex, *pTex, blitRegion, EFilterMode::eLinear);
					// this cmd will wait until the blit operation is complete
					pCmd.ImageBarrier(data.second, EResourceBarrier::efCopy_Source, EResourceBarrier::efShader_Resource, i - 1);

					// down-scaling
					if (mipWidth > 1)
						mipWidth /= 2;
					if (mipHeight > 1)
						mipHeight /= 2;
				}
				pCmd.ImageBarrier(data.second, EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource, pTex->GetNumMipmap() - 1);
			}
			else
			{
				bufferCI.type = EBufferType::efTransfer_Src;
				bufferCI.memoryUsage = EGPUMemoryUsage::eCPU_To_GPU;

				stagingBuffers.emplace_back(VulkanBuffer::Create(*mpContext, bufferCI)); // create a staging buffer
				stagingBuffers[index]->UploadData(bufferCI.pInitData);

				vector<TextureCopyRegion> copyRegions;
				UInt32 offset = 0;
				for (UInt32 face = 0; face < data.second->GetNumArray(); ++face)
				{
					for (UInt32 i = 0; i < data.second->GetNumMipmap(); ++i)
					{
						TextureCopyRegion copyRegion = {};
						copyRegion.mipLevel = i;
						copyRegion.baseArray = face;
						copyRegion.layer = 1;
						copyRegion.width = data.second->GetWidth() >> i;
						copyRegion.height = data.second->GetHeight() >> i;
						copyRegion.depth = 1;
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
				pCmd.ImageBarrier(pTex, EResourceBarrier::efUndefined, EResourceBarrier::efCopy_Dest);
				pCmd.CopyBufferToImage(*stagingBuffers[index], *pTex, copyRegions);
				// transfer complete, switch this texture to a shader read-only texture
				pCmd.ImageBarrier(pTex, EResourceBarrier::efCopy_Dest, EResourceBarrier::efShader_Resource);
			}

			++index;
			if (pTex->GetNumArray() == 6) // cubemap resource destroy
				ktxTexture_Destroy(reinterpret_cast<ktxTexture*>(pTex->GetUserData()));
		}
		pCmd.EndRecord();

		mpContext->pGraphicQueue->SubmitCommands<0, 0, 0>(&pCmd, nullptr, nullptr, nullptr, nullptr);
		mpContext->pGraphicQueue->WaitIdle(); // [Critical] Huge impact on performance 

		mpContext->pGraphicCommandPool->FreeCommandBuffer(pCmd);
		for (auto* e : stagingBuffers)
			Delete(e);
		mWaitToSubmitTextures.clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// DescriptorSet	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	void VulkanResourceRegistry::AddDescriptorSet(const string& name, VulkanDescriptorSet* pSet, bool bCreateHandle)
	{
		SG_PROFILE_FUNCTION();

		auto node = mDescriptorSets.find(name);
		if (node != mDescriptorSets.end())
		{
			SG_LOG_ERROR("Already have a descriptor set named: %s", name.c_str());
			return;
		}
		mDescriptorSets[name] = pSet;

		if (bCreateHandle)
			AddDescriptorSetHandle(name, pSet);
	}

	void VulkanResourceRegistry::AddDescriptorSetHandle(const string& name, VulkanDescriptorSet* pSet)
	{
		SG_PROFILE_FUNCTION();

		auto node = mDescriptorSetHandles.find(name);
		if (node != mDescriptorSetHandles.end())
		{
			SG_LOG_ERROR("Already have a descriptor set handle named: %s", name.c_str());
			return;
		}
		mDescriptorSetHandles[name] = Handle<VulkanDescriptorSet*>(pSet, nullptr);
	}

	void VulkanResourceRegistry::RemoveDescriptorSet(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto node = mDescriptorSets.find(name);
		if (node == mDescriptorSets.end())
		{
			SG_LOG_ERROR("No descriptor set named: %s", name.c_str());
			return;
		}

		Delete(node->second);
		mDescriptorSets.erase(node);

		// because the data had been destroyed, the handle must be invalid.
		auto nodeHandle = mDescriptorSetHandles.find(name);
		if (nodeHandle != mDescriptorSetHandles.end())
		{
			auto& handle = nodeHandle->second;
			handle.Invalidate();
			mDescriptorSetHandles.erase(nodeHandle);
		}
	}

	void VulkanResourceRegistry::RemoveDescriptorSet(VulkanDescriptorSet* pSet)
	{
		SG_PROFILE_FUNCTION();

		for (auto node : mDescriptorSets)
		{
			if (node.second == pSet)
			{
				mDescriptorSets.erase(node.first);
				for (auto nodeHandle : mDescriptorSetHandles)
				{
					if (nodeHandle.second.GetData() == pSet)
					{
						auto& handle = nodeHandle.second;
						handle.Invalidate();
						mDescriptorSetHandles.erase(nodeHandle.first);
					}
					return;
				}
				return;
			}
		}
		SG_LOG_ERROR("No descriptor set: 0x%p", pSet);
	}

	VulkanDescriptorSet* VulkanResourceRegistry::GetDescriptorSet(const string& name) const
	{
		SG_PROFILE_FUNCTION();

		auto node = mDescriptorSets.find(name);
		if (node == mDescriptorSets.end())
		{
			SG_LOG_ERROR("No descritor set named: %s", name.c_str());
			return nullptr;
		}
		return node->second;
	}

	Handle<VulkanDescriptorSet*> VulkanResourceRegistry::GetDescriptorSetHandle(const string& name)
	{
		SG_PROFILE_FUNCTION();

		auto node = mDescriptorSetHandles.find(name);
		if (node == mDescriptorSetHandles.end())
		{
			SG_LOG_ERROR("No descritor set handle named: %s", name.c_str());
			return Handle<VulkanDescriptorSet*>(nullptr, nullptr);
		}
		return node->second;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// RenderTargets	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateRenderTarget(const TextureCreateDesc& textureCI, bool isDepth)
	{
		SG_PROFILE_FUNCTION();

		if (mRenderTargets.count(textureCI.name) != 0)
		{
			SG_LOG_ERROR("Already have a render target named %s!", textureCI.name);
			return false;
		}

		mRenderTargets[textureCI.name] = VulkanRenderTarget::Create(*mpContext, textureCI, isDepth);
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
		SG_PROFILE_FUNCTION();

		auto* pRt = GetRenderTarget(name);
		if (pRt)
		{
			Delete(pRt);
			mRenderTargets.erase(name);
		}
	}

	VulkanRenderTarget* VulkanResourceRegistry::GetRenderTarget(const string& name) const
	{
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

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
		SG_PROFILE_FUNCTION();

		if (mSamplers.count(name) == 0)
		{
			SG_LOG_ERROR("No sampler named: %s", name);
			return nullptr;
		}
		return mSamplers[name];
	}

}