#include "ArcdpsExtension.h"

void ArcdpsExtension::Setup(HMODULE pDll, ID3D11Device* pD11Device, ImGuiContext* pImGuiContext) {
	IconLoader::instance().Setup(pDll, pD11Device);
	Localization::instance();
	SimpleNetworkStack::instance();

	// needs imgui
#if ARCDPS_EXTENSION_IMGUI
	PositioningComponentImGuiHook::InstallHooks(pImGuiContext);
#endif

	// needs UE
#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
	KeyBindHandler::instance();
#endif
}

void ArcdpsExtension::Shutdown() {
	g_singletonManagerInstance.Shutdown();
}