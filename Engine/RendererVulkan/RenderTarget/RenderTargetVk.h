#pragma once

#include "Render/RenderTarget.h"
#include "Render/SwapChain.h"

namespace SG
{

	struct Renderer;
	class RenderTargetVk : public RenderTarget
	{
	public:
		RenderTargetVk(Renderer* pRenderer, const Resolution& res, EImageFormat format);
		~RenderTargetVk();

		virtual Resolution GetResolution() const override;
	private:
		Renderer* mpRenderer = nullptr;
	};

}