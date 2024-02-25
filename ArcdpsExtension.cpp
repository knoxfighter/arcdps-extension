#include "ArcdpsExtension.h"

#include "IconLoader.h"
#include "Localization.h"
#include "SimpleNetworkStack.h"

#if ARCDPS_EXTENSION_IMGUI
#include "Windows/PositioningComponent.h"
#endif

#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
#include "KeyBindHandler.h"
#endif

#if ARCDPS_EXTENSION_IMGUI
void ArcdpsExtension::Setup(HMODULE pDll, ID3D11Device* pD11Device, ImGuiContext* pImGuiContext) {
#else
void ArcdpsExtension::Setup(HMODULE pDll, ID3D11Device* pD11Device) {
#endif
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