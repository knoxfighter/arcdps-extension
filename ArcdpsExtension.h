#pragma once

#include "IconLoader.h"
#include "KeyBindHandler.h"
#include "Localization.h"
#include "SimpleNetworkStack.h"
#include "Windows/PositioningComponent.h"

namespace ArcdpsExtension {
	void Setup(HMODULE pDll, ID3D11Device* pD11Device, ImGuiContext* pImGuiContext);

	void Shutdown();
} // namespace ArcdpsExtension
