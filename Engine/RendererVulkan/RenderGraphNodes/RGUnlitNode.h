#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Render/Camera/ICamera.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

// TODO: remove it
#include "volk.h"

#include "Stl/vector.h"

namespace SG
{

	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanPipeline;
	class VulkanPipelineLayout;
	class VulkanDescriptorSetLayout;

	class Geometry;

	class RGUnlitNode final : public RenderGraphNode
	{
	public:
		RGUnlitNode(VulkanContext& context);
		~RGUnlitNode();

		void BindGeometry(const char* name);
		void SetCamera(ICamera* pCamera);
	private:
		virtual void Reset() override;
		virtual void Prepare(VulkanRenderPass* pRenderpass) override;
		virtual void Update(float deltaTime, UInt32 frameIndex) override;
		virtual void Draw(RGDrawContext& context) override;
	private:
		VulkanContext&        mContext;

		LoadStoreClearOp      mColorRtLoadStoreOp;
		LoadStoreClearOp      mDepthRtLoadStoreOp;

		VulkanDescriptorSetLayout* mpUBOSetLayout;
		VulkanPipelineLayout*      mpPipelineLayout;
		VulkanPipeline*            mpPipeline;
		Shader                     mBasicShader;
		Geometry*                  mpGeometry;

		ICamera* mpCamera;
		// Temporary
		Vector3f mModelPosition;
		float    mModelScale;
		Vector3f mModelRotation;

		struct SG_ALIGN(64) UBO
		{
			Matrix4f view;
			Matrix4f proj;
			Vector3f viewPos;
			float    pad;
		};
		UBO      mCameraUBO;

		struct SG_ALIGN(64) PushConstant
		{
			Matrix4f model;
			Matrix4f inverseTransposeModel;
		};
		PushConstant mPushConstant;

		//vector<eastl::pair<UInt32, VkDescriptorSet>> mDescriptorSets;
		//struct BindConstantData
		//{
		//	EShaderStage stage;
		//	UInt32       size;
		//	void*        pData;

		//	BindConstantData(EShaderStage s, UInt32 sz, void* ptr)
		//		:stage(s), size(sz), pData(ptr)
		//	{}
		//};
		//vector<BindConstantData> mPushConstants;
	};

}