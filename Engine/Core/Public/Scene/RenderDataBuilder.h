#pragma once

#include "Scene/Scene.h"
#include "Render/CommonRenderData.h"
#include "Render/Buffer.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"
#include "eastl/unordered_map.h"

namespace SG
{

	struct RenderMeshBuildData
	{
		UInt32 objectId = UInt32(-1);
		UInt32 instanceCount = 1;
		vector<PerInstanceData> perInstanceData = {};
	};

	interface IAsset;

	class RenderDataBuilder
	{
	public:
		RenderDataBuilder() = default;
		RenderDataBuilder(WeakRefPtr<Scene> pScene);

		void SetScene(WeakRefPtr<Scene> pScene);
		void LoadInNeccessaryDataFromDisk();
		void ResolveRenderData();

		const eastl::unordered_map<UInt32, WeakRefPtr<IAsset>>& GetAssets() const noexcept { return mAssets; }

		template <typename Func>
		void TraverseRenderData(Func&& func);
	private:
		WeakRefPtr<Scene> mpScene;
		eastl::unordered_map<UInt32, RenderMeshBuildData> mRenderMeshBuildDataMap; // meshId -> RenderMeshBuildData
		eastl::unordered_map<UInt32, WeakRefPtr<IAsset>> mAssets; // assetId -> IAsset

		bool mbIsRenderDataReady = false;
	};

	template <typename Func>
	void RenderDataBuilder::TraverseRenderData(Func&& func)
	{
		if (!mbIsRenderDataReady)
		{
			SG_LOG_WARN("Please build the render data!");
			return;
		}

		for (auto node : mRenderMeshBuildDataMap)
		{
			func(node.first, node.second);
		}
	}

}