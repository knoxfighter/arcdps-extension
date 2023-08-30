#pragma once

#include "IconLoader.h"
#include "KeyBindHandler.h"
#include "Localization.h"
#include "SimpleNetworkStack.h"
#include "Windows/PositioningComponent.h"

namespace ArcdpsExtension {
	void Setup(HMODULE pDll, ID3D11Device* pD11Device, ImGuiContext* pImGuiContext) {
		IconLoader::instance().Setup(pDll, pD11Device);
		KeyBindHandler::instance();
		Localization::instance();
		SimpleNetworkStack::instance();
		PositioningComponentImGuiHook::InstallHooks(pImGuiContext);
	}
	void Shutdown() {
		g_singletonManagerInstance.Shutdown();
	}
} // namespace ArcdpsExtension
