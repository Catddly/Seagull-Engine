#pragma once

#include "Render/Shader.h"

#include "RendererVulkan/Backend/VulkanBuffer.h"

#include "Stl/vector.h"
#include "Stl/string.h"

namespace SG
{

	class Geometry
	{
	public:
		Geometry(const char* name, float* pVerticies, UInt32 numVertex, UInt32* pIndices, UInt32 numIndex);
		Geometry(const char* name, float* pVerticies, UInt32 numVertex, UInt16* pIndices, UInt16 numIndex);

		VulkanBuffer* GetVertexBuffer() const;
		VulkanBuffer* GetIndexBuffer() const;

		~Geometry() = default;
	private:
		const char*   mName;
	};

}