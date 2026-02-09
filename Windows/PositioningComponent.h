#pragma once

#include "../arcdps_structs.h"
#include "ComponentBase.h"
#include "MainWindow.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <vector>

namespace ArcdpsExtension {
	class PositioningComponent : public ComponentBase {
	public:
		explicit PositioningComponent(MainWindow* pMainWindow);

		void PreDrawHookFunction(ImGuiWindowFlags& pFlags);

		/**
		 * This is called by the `PositioningComponentImGuiHook` below.
		 */
		void Reposition();
		~PositioningComponent() override;

	protected:
		virtual Position& getPositionMode() = 0;
		virtual CornerPosition& getCornerPosition() = 0;
		virtual ImVec2& getCornerVector() = 0;
		virtual CornerPosition& getAnchorPanelCorner() = 0;
		virtual CornerPosition& getSelfPanelCorner() = 0;
		virtual ImGuiID& getFromWindowId() = 0;

		/**
		 * This is called to draw the SubMenu of "Position".
		 * This will draw the "Position
		 */
		virtual void DrawPositionSettingsSubMenu();
	};

	class PositioningComponentImGuiHook {
	public:
		/**
		 * This has to be called in `get_init_addr` or `mod_init` with the ImGuiContext arcdps gave you.
		 */
		static void InstallHooks(ImGuiContext* imGuiContext);

		/**
		 * A vector with all `MainWindow`s of the plugin you use (not others).
		 * MainWindow will add itself to this in it's constructor
		 */
		static inline std::vector<PositioningComponent*> POSITIONING_COMPONENTS;

		/**
		 * buffer for all ImGui windows that a MainWindow can be attached to.
		 * Will be filled and cleared automatically.
		 */
		static inline std::vector<ImGuiID> ANCHORING_HIGHLIGHTED_WINDOWS;

	private:
		/**
		 * These functions are called by ImGui itself, don't worry about them
		 */
		static void PostNewFrame(ImGuiContext* pImguiContext, ImGuiContextHook*);
		static void PreEndFrame(ImGuiContext* pImguiContext, ImGuiContextHook*);
	};
} // namespace ArcdpsExtension
