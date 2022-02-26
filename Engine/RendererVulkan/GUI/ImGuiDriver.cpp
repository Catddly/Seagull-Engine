#include "StdAfx.h"
#include "ImGuiDriver.h"

#include "Platform/OS.h"
#include "System/Logger.h"
#include "Memory/Memory.h"

#include "imgui/imgui.h"

namespace SG
{

	static void* _ImGuiMemAllocFunc(Size sz, void* userData)
	{
		SG_NO_USE(userData);
		return Memory::Malloc(sz);
	}

	static void  _ImGuiMemFreeFunc(void* ptr, void* userData)
	{
		SG_NO_USE(userData);
		return Memory::Free(ptr);
	}

	bool ImGuiDriver::OnInit()
	{
		ImGui::SetAllocatorFunctions(_ImGuiMemAllocFunc, _ImGuiMemFreeFunc);
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // enable Multi-Viewport / Platform Windows

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;  // We can create multi-viewports on the Renderer side (optional)

		ImGui::StyleColorsDark();

		// when viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Our mouse update function expect PlatformHandle to be filled for the main viewport
		ImGuiViewport* mainViewport = ImGui::GetMainViewport();
		mainViewport->PlatformHandle = (void*)OperatingSystem::GetMainWindow();
		mainViewport->PlatformHandleRaw = OperatingSystem::GetMainWindow()->GetNativeHandle();

		return true;
	}

	void ImGuiDriver::OnShutdown()
	{
		ImGui::DestroyContext();
	}

	void ImGuiDriver::OnDraw()
	{
		ImGuiIO& io = ImGui::GetIO();

		io.MousePos = { (float)OperatingSystem::GetMousePos()[0], (float)OperatingSystem::GetMousePos()[1] };

		io.DisplaySize = { (float)OperatingSystem::GetMainWindow()->GetWidth(), (float)OperatingSystem::GetMainWindow()->GetHeight() };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };
		
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Monitors.resize(0);

		ImGuiPlatformMonitor monitor;
		Rect& monitorRect = OperatingSystem::GetMainMonitor()->GetMonitorRect();
		monitor.MainPos = monitor.WorkPos = { (float)monitorRect.left, (float)monitorRect.top };
		monitor.MainSize = monitor.WorkSize = { (float)GetRectWidth(monitorRect), (float)GetRectHeight(monitorRect) };
		platformIO.Monitors.push_back(monitor);
	}

}