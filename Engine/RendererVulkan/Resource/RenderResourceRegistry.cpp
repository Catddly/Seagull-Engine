#include "StdAfx.h"
#include "RenderResourceRegistry.h"

#include "System/System.h"
#include "System/Logger.h"
#include "Memory/Memory.h"
#include "Render/SwapChain.h"
#include "Scene/Mesh/MeshDataArchive.h"
#include "TipECS/Entity.h"

#include "RendererVulkan/Backend/VulkanContext.h"
#include "RendererVulkan/Backend/VulkanBuffer.h"
#include "RendererVulkan/Backend/VulkanSwapchain.h"
#include "RendererVulkan/Backend/VulkanSynchronizePrimitive.h"

#include "ktx/ktx.h"

namespace SG
{

	void VulkanResourceRegistry::Initialize(const VulkanContext* pContext)
	{
		mpContext = const_cast<VulkanContext*>(pContext);

		BufferCreateDesc ssboCI = {};
		ssboCI.name = "perObjectBuffer";
		ssboCI.bufferSize = sizeof(ObjcetRenderData) * SG_MAX_NUM_OBJECT;
		ssboCI.type = EBufferType::efStorage;
		CreateBuffer(ssboCI);

		// This is only for debuging purpose
		//ssboCI = {};
		//ssboCI.name = "cullingOutputData";
		//ssboCI.bufferSize = sizeof(CullingOutputData) * SG_MAX_NUM_OBJECT;
		//ssboCI.type = EBufferType::efStorage;
		//CreateBuffer(ssboCI);

		BufferCreateDesc cullBufferCI = {};
		cullBufferCI.name = "cullUbo";
		cullBufferCI.bufferSize = sizeof(GPUCullUBO) * SG_MAX_NUM_OBJECT;
		cullBufferCI.type = EBufferType::efUniform;
		CreateBuffer(cullBufferCI);

		auto pSkybox = SSystem()->GetMainScene()->GetSkyboxEntity();
		auto& mesh = pSkybox.GetComponent<MeshComponent>();
		auto& skyboxVertices = MeshDataArchive::GetInstance()->GetData(mesh.meshId)->vertices;
		const UInt64 vbSize = skyboxVertices.size() * sizeof(float);

		BufferCreateDesc bufferCI = {};
		bufferCI.name = "skybox_vb";
		bufferCI.pInitData = skyboxVertices.data();
		bufferCI.bufferSize = static_cast<UInt32>(vbSize);
		bufferCI.type = EBufferType::efVertex;
		CreateBuffer(bufferCI, true);
		FlushBuffers();

		// eliminate the translation part of the matrix
		ObjcetRenderData renderData = { Matrix4f(Matrix3f(SSystem()->GetMainScene()->GetMainCamera()->GetViewMatrix())) };
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
		auto* pCamera = pScene->GetMainCamera();
		cameraUbo.proj = pCamera->GetProjMatrix();

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
		auto& skyboxUbo = GetSkyboxUBO();
		skyboxUbo.proj = cameraUbo.proj;

		auto& compositionUbo = GetCompositionUBO();
		compositionUbo.gamma = 2.2f;
		compositionUbo.exposure = 1.0f;

		auto* pSSBOObject = GetBuffer("perObjectBuffer");
		// update all the render data of the render mesh
		auto meshes = pScene->View<TransformComponent, MeshComponent, MaterialComponent>();
		for (auto& entity : meshes)
		{
			auto [trans, mesh, mat] = entity.GetComponent<TransformComponent, MeshComponent, MaterialComponent>();

			ObjcetRenderData renderData;
			renderData.model = GetTransform(trans);
			renderData.inverseTransposeModel = glm::transpose(glm::inverse(renderData.model));
			renderData.meshId = mesh.meshId;
			renderData.MRIF = { mat.metallic, mat.roughness, MeshDataArchive::GetInstance()->HaveInstance(renderData.meshId) ? 1.0f : -1.0f };
			pSSBOObject->UploadData(&renderData, sizeof(ObjcetRenderData), sizeof(ObjcetRenderData) * mesh.objectId);
		}

		// update all the lights
		//auto lights = pScene->View<LightTag>();
		//for (auto& entity : lights)
		//{
		//	if (entity.HasComponent<DirectionalLightComponent>())
		//	{
		//		auto& lightUbo = GetLightUBO();
		//		auto& shadowUbo = GetShadowUBO();
		//		auto [trans, dl] = entity.GetComponent<TransformComponent, DirectionalLightComponent>();
		//		shadowUbo.lightSpaceVP = CalcDirectionalLightViewProj(trans);
		//		lightUbo.lightSpaceVP = shadowUbo.lightSpaceVP;
		//		lightUbo.directionalColor = { dl.color, 1.0f };
		//		lightUbo.viewDirection = CalcViewDirectionNormalized(trans);
		//	}
		//}
	}

	void VulkanResourceRegistry::Shutdown()
	{
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

		// update camera data
		auto* pCamera = pLScene->GetMainCamera();
		if (pCamera->IsViewDirty())
		{
			Frustum cameraFrustum = pCamera->GetFrustum();
			auto& cullUbo = GetGPUCullUBO();
			cullUbo.frustum[0] = cameraFrustum.GetTopPlane();
			cullUbo.frustum[1] = cameraFrustum.GetBottomPlane();
			cullUbo.frustum[2] = cameraFrustum.GetLeftPlane();
			cullUbo.frustum[3] = cameraFrustum.GetRightPlane();
			cullUbo.frustum[4] = cameraFrustum.GetFrontPlane();
			cullUbo.frustum[5] = cameraFrustum.GetBackPlane();
			cullUbo.viewPos = pCamera->GetPosition();
			UpdataBufferData("cullUbo", &cullUbo);

			auto& skyboxUbo = GetSkyboxUBO();
			auto& cameraUbo = GetCameraUBO();
			cameraUbo.viewPos = pCamera->GetPosition();
			cameraUbo.view = pCamera->GetViewMatrix();
			skyboxUbo.model = Matrix4f(Matrix3f(cameraUbo.view)); // eliminate the translation part of the matrix
			cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;
			UpdataBufferData("cameraUbo", &cameraUbo);
			UpdataBufferData("skyboxUbo", &skyboxUbo);
			pCamera->ViewBeUpdated();
		}

		// update light data
		auto lights = pLScene->View<LightTag>();
		for (auto entity : lights)
		{
			if (entity.HasComponent<PointLightComponent>())
			{
				auto [tag, trans, light] = entity.GetComponent<TagComponent, TransformComponent, PointLightComponent>();
				if (tag.bDirty)
				{
					auto& lightUbo = GetLightUBO();
					lightUbo.pointLightColor = light.color;
					lightUbo.pointLightRadius = light.radius;
					lightUbo.pointLightPos = trans.position;
					UpdataBufferData("lightUbo", &lightUbo);
					tag.bDirty = false;
				}
			}
			else if (entity.HasComponent<DirectionalLightComponent>())
			{
				auto [tag, trans, light] = entity.GetComponent<TagComponent, TransformComponent, DirectionalLightComponent>();
				if (tag.bDirty)
				{
					auto& shadowUbo = GetShadowUBO();
					auto& lightUbo = GetLightUBO();
					shadowUbo.lightSpaceVP = CalcDirectionalLightViewProj(trans);
					lightUbo.lightSpaceVP = shadowUbo.lightSpaceVP;
					lightUbo.directionalColor = { light.color, 1.0f };
					lightUbo.viewDirection = CalcViewDirectionNormalized(trans);
					UpdataBufferData("shadowUbo", &shadowUbo);
					UpdataBufferData("lightUbo", &lightUbo);
					tag.bDirty = false;
				}
			}

		}

		auto& compositionUbo = GetCompositionUBO();
		UpdataBufferData("compositionUbo", &compositionUbo);
	}

	void VulkanResourceRegistry::WindowResize()
	{
		auto pScene = SSystem()->GetMainScene();
		auto* window = OperatingSystem::GetMainWindow();
		const float  ASPECT = window->GetAspectRatio();
		pScene->GetMainCamera()->SetPerspective(60.0f, ASPECT, 0.01f, 256.0f);

		auto& skyboxUbo = GetSkyboxUBO();
		auto& cameraUbo = GetCameraUBO();
		cameraUbo.proj = pScene->GetMainCamera()->GetProjMatrix();
		skyboxUbo.proj = cameraUbo.proj;
		cameraUbo.viewProj = cameraUbo.proj * cameraUbo.view;

		Frustum cameraFrustum = pScene->GetMainCamera()->GetFrustum();
		auto& cullUbo = GetGPUCullUBO();
		cullUbo.frustum[0] = cameraFrustum.GetTopPlane();
		cullUbo.frustum[1] = cameraFrustum.GetBottomPlane();
		cullUbo.frustum[2] = cameraFrustum.GetLeftPlane();
		cullUbo.frustum[3] = cameraFrustum.GetRightPlane();
		cullUbo.frustum[4] = cameraFrustum.GetFrontPlane();
		cullUbo.frustum[5] = cameraFrustum.GetBackPlane();
		UpdataBufferData("cullUbo", &cullUbo);

		VK_RESOURCE()->UpdataBufferData("cameraUbo", &cameraUbo);
		VK_RESOURCE()->UpdataBufferData("skyboxUbo", &skyboxUbo);
	}

	VulkanResourceRegistry* VulkanResourceRegistry::GetInstance()
	{
		static VulkanResourceRegistry instance;
		return &instance;
	}

	void VulkanResourceRegistry::WaitBuffersUpdate() const
	{
		for (auto* pFence : mpBufferUploadFences)
			pFence->WaitAndReset();

		for (auto& buf : mSubmitedCommandBuffers)
			mpContext->transferCommandPool->FreeCommandBuffer(buf);
		for (auto* pStagingBuf : mpStagingBuffers)
			Memory::Delete(pStagingBuf);
		for (auto* pFence : mpBufferUploadFences)
			Memory::Delete(pFence);

		mpBufferUploadFences.clear();
		mpStagingBuffers.clear();
		mSubmitedCommandBuffers.clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// Buffers
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool VulkanResourceRegistry::CreateBuffer(const BufferCreateDesc& bufferCI, bool bLocal)
	{
		if (mBuffers.count(bufferCI.name) != 0) // do data copy
		{
			if (bLocal)
			{
				if (!bufferCI.pInitData)
				{
					SG_LOG_ERROR("Device local buffer must have initialize data!");
					return false;
				}
				// make a copy of bufferCI
				mWaitToSubmitBuffers.push_back({ bufferCI, GetBuffer(bufferCI.name) });
				return true;
			}
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
		{
			if (bufferCI.bSubBufer)
				pBuffer->UploadData(bufferCI.pInitData, bufferCI.dataSize, bufferCI.dataOffset);
			else
				pBuffer->UploadData(bufferCI.pInitData);
		}

		if (bLocal)
		{
			if (bufferCI.pInitData)
			{
				// make a copy of bufferCI
				mWaitToSubmitBuffers.push_back({ bufferCI, pBuffer });
			}
		}
		return true;
	}

	void VulkanResourceRegistry::FlushBuffers() const
	{
		if (mWaitToSubmitBuffers.empty())
			return;

		auto& cmd = mSubmitedCommandBuffers.push_back();
		mpContext->transferCommandPool->AllocateCommandBuffer(cmd);
		mpBufferUploadFences.push_back(VulkanFence::Create(mpContext->device));

		cmd.BeginRecord();
		UInt32 index = 0;
		for (auto& data : mWaitToSubmitBuffers) // copy all the buffers data using staging buffer
		{
			data.first.type = EBufferType::efTransfer_Src;
			if (data.first.bSubBufer)
				data.first.bufferSize = data.first.dataSize;

			mpStagingBuffers.push_back(VulkanBuffer::Create(mpContext->device, data.first, false));
			auto* pStagingBuf = mpStagingBuffers.back();
			pStagingBuf->UploadData(data.first.pInitData);

			cmd.CopyBuffer(*pStagingBuf, *data.second, 0, data.first.bSubBufer ? data.first.dataOffset : 0);
			++index;
		}
		cmd.EndRecord();

		mpContext->transferQueue.SubmitCommands(&cmd, nullptr, nullptr, mpBufferUploadFences.back());
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
			bufferCI.bufferSize = textureCI.sizeInByte;
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