#pragma once

#include <d3d11.h>
#include <Windows.h>

#if ARCDPS_EXTENSION_IMGUI
#include <imgui/imgui.h>
#endif

namespace ArcdpsExtension {
#if ARCDPS_EXTENSION_IMGUI
	void Setup(HMODULE pDll, ID3D11Device* pD11Device, ImGuiContext* pImGuiContext);
#else
	void Setup(HMODULE pDll, ID3D11Device* pD11Device);
#endif

	void Shutdown();
} // namespace ArcdpsExtension
