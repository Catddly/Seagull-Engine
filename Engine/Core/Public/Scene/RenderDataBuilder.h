#pragma once

#include "Scene/Scene.h"
#include "Render/CommonRenderData.h"
#include "Render/Buffer.h"

#include "Stl/vector.h"
#include "Stl/SmartPtr.h"
#include "eastl/unordered_map.h"
#include "eastl/map.h"

namespace SG
{

	struct RenderMeshBuildData
	{
		UInt32 objectId = UInt32(-1); //! Object id to get the ObjcetRenderData.
		UInt32 instanceCount = 1; //! Instance count of this draw call, used to decide whether this draw call should be instance draw.
		vector<PerInstanceData> perInstanceData = {}; // Object ids of instance data.
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

		void OnUpdate(float deltaTime);

		const eastl::unordered_map<UInt32, WeakRefPtr<IAsset>>& GetCurrentFrameNewAssets() const noexcept { return mCurrentFrameNewAssets; }
		const eastl::unordered_map<UInt32, WeakRefPtr<IAsset>>& GetAssets() const noexcept { return mAssets; }

		template <typename Func>
		void TraverseRenderData(Func&& func);
	private:
		void LogDebugInfo() const;
	private:
		WeakRefPtr<Scene> mpScene;
		eastl::map<UInt32, RenderMeshBuildData> mRenderMeshBuildDataMap; // meshId -> RenderMeshBuildData (ordered with meshId)
		eastl::unordered_map<UInt32, WeakRefPtr<IAsset>> mCurrentFrameNewAssets; // assetId -> IAsset
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