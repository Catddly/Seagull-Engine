#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Render/Camera/ICamera.h"

#include "RendererVulkan/RenderGraph/RenderGraphNode.h"

#include "volk.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"

namespace SG
{

	class VulkanContext;

	class VulkanRenderTarget;
	class VulkanShader;
	class VulkanPipeline;
	class VulkanPipelineSignature;

	class Geometry;

	class RGDefaultNode final : public RenderGraphNode
	{
	public:
		RGDefaultNode(VulkanContext& context);
		~RGDefaultNode();

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

		RefPtr<VulkanPipelineSignature> mpPipelineSignature;
		VulkanPipeline*                 mpPipeline;
		RefPtr<VulkanShader>            mpShader;
		Geometry*                       mpGeometry;

		ICamera* mpCamera;
		// Temporary
		Vector3f mModelPosition;
		float    mModelScale;
		Vector3f mModelRotation;

		struct UBO
		{
			Matrix4f view;
			Matrix4f proj;
			Vector3f viewPos;
			float    pad;
		};
		UBO      mCameraUBO;

		struct PushConstant
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