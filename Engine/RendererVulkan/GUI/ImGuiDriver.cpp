#include "StdAfx.h"
#include "ImGuiDriver.h"

#include "System/Logger.h"

#include "imgui/imgui.h"

namespace SG
{

	bool ImGuiDriver::OnInit()
	{
		mpImGuiContext = ImGui::CreateContext();
		if (!mpImGuiContext)
		{
			SG_LOG_ERROR("Failed to create imgui context!");
			return false;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		// when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	}

	void ImGuiDriver::OnShutdown()
	{
		ImGui::DestroyContext(mpImGuiContext);
	}

	void ImGuiDriver::OnDraw()
	{

	}

}