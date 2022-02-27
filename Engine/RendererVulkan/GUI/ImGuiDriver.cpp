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

	static void _ImGui_Platform_CreateWindow_Impl(ImGuiViewport* vp)
	{
		auto* monitor = OperatingSystem::GetMainMonitor();
		auto* newWindow = Memory::New<Window>(monitor);

		vp->PlatformHandle = newWindow;
		vp->PlatformHandleRaw = newWindow->GetNativeHandle();
	}

	static void _ImGui_Platform_DestroyWindow_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		if (pWindow)
			Memory::Delete(pWindow);
		vp->PlatformHandle = nullptr;
		vp->PlatformHandleRaw = nullptr;
	}

	static void _ImGui_Platform_ShowWindow_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->ShowWindow();
	}

	static void _ImGui_Platform_SetWindowPos_Impl(ImGuiViewport* vp, ImVec2 pos)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetPosition((UInt32)pos.x, (UInt32)pos.y);
	}

	static ImVec2 _ImGui_Platform_GetWindowPos_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		auto& rect = pWindow->GetCurrRect();
		return { (float)rect.left, (float)rect.top };
	}

	static void _ImGui_Platform_SetWindowSize_Impl(ImGuiViewport* vp, ImVec2 size)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetSize((UInt32)size.x, (UInt32)size.y);
	}

	static ImVec2 _ImGui_Platform_GetWindowSize_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return { (float)pWindow->GetWidth(), (float)pWindow->GetHeight() };
	}

	static void _ImGui_Platform_SetWindowFocus_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetFocus();
	}

	static bool _ImGui_Platform_GetWindowFocus_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return pWindow->IsFocus();
	}

	static bool _ImGui_Platform_GetWindowMinimized_Impl(ImGuiViewport* vp)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		return pWindow->IsMinimize();
	}

	static void _ImGui_Platform_SetWindowTitle_Impl(ImGuiViewport* vp, const char* str)
	{
		Window* pWindow = reinterpret_cast<Window*>(vp->PlatformHandle);
		pWindow->SetTitle(str);
	}

	static void _ImGuiBindWindowPlatformFunc()
	{
		// Register platform interface (will be coupled with a renderer interface)
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_CreateWindow = _ImGui_Platform_CreateWindow_Impl;
		platformIO.Platform_DestroyWindow = _ImGui_Platform_DestroyWindow_Impl;
		platformIO.Platform_ShowWindow = _ImGui_Platform_ShowWindow_Impl;
		platformIO.Platform_SetWindowPos = _ImGui_Platform_SetWindowPos_Impl;
		platformIO.Platform_GetWindowPos = _ImGui_Platform_GetWindowPos_Impl;
		platformIO.Platform_SetWindowSize = _ImGui_Platform_SetWindowSize_Impl;
		platformIO.Platform_GetWindowSize = _ImGui_Platform_GetWindowSize_Impl;
		platformIO.Platform_SetWindowFocus = _ImGui_Platform_SetWindowFocus_Impl;
		platformIO.Platform_GetWindowFocus = _ImGui_Platform_GetWindowFocus_Impl;
		platformIO.Platform_GetWindowMinimized = _ImGui_Platform_GetWindowMinimized_Impl;
		platformIO.Platform_SetWindowTitle = _ImGui_Platform_SetWindowTitle_Impl;
		//platformIO.Platform_RenderWindow = NULL;
		//platformIO.Platform_SwapBuffers = NULL;
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

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Monitors.resize(0);

		ImGuiPlatformMonitor monitor;
		Rect& monitorRect = OperatingSystem::GetMainMonitor()->GetMonitorRect();
		monitor.MainPos = monitor.WorkPos = { (float)monitorRect.left, (float)monitorRect.top };
		monitor.MainSize = monitor.WorkSize = { (float)GetRectWidth(monitorRect), (float)GetRectHeight(monitorRect) };
		platformIO.Monitors.push_back(monitor);

		_ImGuiBindWindowPlatformFunc();

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

		auto& mousePos = OperatingSystem::GetMainWindow()->GetMousePosRelative();
		io.MousePos = { (float)mousePos[0], (float)mousePos[1] };

		io.DisplaySize = { (float)OperatingSystem::GetMainWindow()->GetWidth(), (float)OperatingSystem::GetMainWindow()->GetHeight() };
		io.DisplayFramebufferScale = { 1.0f, 1.0f };
	}

}