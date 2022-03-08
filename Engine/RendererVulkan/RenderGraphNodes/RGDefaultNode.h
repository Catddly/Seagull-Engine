#pragma once

#include "Render/Shader.h"
#include "Render/FrameBuffer.h"
#include "Scene/Camera/ICamera.h"
#include "Scene/Light/PointLight.h"

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

	class VulkanGeometry;

	class RGDefaultNode final : public RenderGraphNode
	{
	public:
		RGDefaultNode(VulkanContext& context);
		~RGDefaultNode();
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
		VulkanGeometry*                 mpGeometry;
		const PointLight*               mpPointLight;

		ICamera* mpCamera;
		// Temporary
		Vector3f mModelPosition;
		float    mModelScale;
		Vector3f mModelRotation;

		struct UBO
		{
			Matrix4f view;
			Matrix4f proj;
			Matrix4f lightSpace;
			Vector3f viewPos;
			float    radius;
			Vector3f position;
			float    pad;
			Vector3f color;
		};
		UBO mUBO;
		
		struct PushConstant
		{
			Matrix4f model;
			Matrix4f inverseTransposeModel;
		};
		PushConstant mPushConstant;
	};

}