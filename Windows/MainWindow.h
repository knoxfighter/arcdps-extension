#pragma once

#include "ComponentBase.h"

#include "../arcdps_structs.h"

#include <functional>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ArcdpsExtension {
	enum MainWindowFlags_ : int {
		MainWindowFlags_None = 0,
		MainWindowFlags_NoStyleMenu = 1 << 0, // Do NOT show the Style submenu in the context menu
	};
	typedef int MainWindowFlags; // -> enum MainWindowFlags_

	class MainWindow {
	public:
		MainWindow() = default;

		virtual ~MainWindow();

		// delete copy/move
		MainWindow(const MainWindow& other) = delete;
		MainWindow(MainWindow&& other) noexcept = delete;
		MainWindow& operator=(const MainWindow& other) = delete;
		MainWindow& operator=(MainWindow&& other) noexcept = delete;

		/**
	 * Draw the Window itself, has to be called in `mod_imgui` to draw the window.
	 * It will only be drawn if `GetOpenVar` returns true.
	 */
		virtual void Draw(ImGuiWindowFlags imGuiWindowFlags = 0, MainWindowFlags mainWindowFlags = 0);

		/**
	 * Call this function in `mod_init`, so the window is fully initialized!
	 */
		virtual void Init();

		/**
	 * Draw a checkbox for this window.
	 * Call it in `mod_windows`.
	 * Check the param of that function yourself.
	 */
		void DrawOptionCheckbox();

		/**
	 * Function to call to make the max-height work.
	 * Call this within your window at a position where the max-height should be.
	 * Multiple calls to this will override the value and only the last call will be used for calculation.
	 *
	 * Depending on where you call this function it is off by a few pixel.
	 * Override it to correct it.
	 * Example: The DemoWindow wants to have `- GImGui->Style.ItemSpacing.y` added and a ImGui::Table wants to have the inline border added.
	 */
		virtual void SetMaxHeightCursorPos(float pNewCursorPos = ImGui::GetCursorPosY());

		virtual bool& GetOpenVar() = 0;

		ImGuiWindow* GetInnerWindow();

		template<typename T>
		requires std::is_base_of_v<ComponentBase, T>
		void CreateComponent() {
			mComponents.emplace_back(new T(this));
		}

		typedef std::function<void(ImGuiWindowFlags&)> PreDrawHookFunction;
		void RegisterPreDrawHook(PreDrawHookFunction pFun);

		typedef std::function<void()> ContextMenuHookFunction;
		void RegisterContextMenuHook(ContextMenuHookFunction pFun);

		typedef std::function<void()> DrawContentHookFunction;
		void RegisterContentHook(DrawContentHookFunction pFun);

		typedef std::function<void()> InitHookFunction;
		void RegisterInitHook(InitHookFunction pFun);

		typedef std::function<void()> DrawStyleSubMenuHookFunction;
		void RegisterDrawStyleSubMenuHook(DrawStyleSubMenuHookFunction pFun);

		/**
	 * This has to be `true` to show the scrollbar.
	 * Therefore setting it to `true` by default is recommended.
	 * A scrollbar is only shown when the window is bigger than it's content.
	 */
		virtual bool& GetShowScrollbar() = 0;

		float GetMaxCursorPos();

	protected:
		ImGuiWindow* mThisWindow = nullptr;

		/**
	 * Subcall inside context menu to add more things to it.
	 * Will be called at the beginning of the ContextMenu.
	 */
		virtual void DrawContextMenu() = 0;

		/**
	 * Called when the content of the Window should be drawn.
	 * This is the main function to draw into the window.
	 */
		virtual void DrawContent() = 0;

		/**
	 * Used to draw additional settings below the sizingPolicy combobox.
	 * Example: Tables might have a specific column size depending on the sizingPolicy.
	 */
		virtual void DrawSizingPolicySubSettings(SizingPolicy sizingPolicy) {}

		virtual SizingPolicy& getSizingPolicy() = 0;

		/**
	 * This has to be `true` to show the titlebar.
	 * Therefore setting it to `true` by default is recommended.
	 */
		virtual bool& getShowTitleBar() = 0;

		/**
	 * The name of the window (shown in the title bar) and changeable by the user.
	 */
		virtual std::optional<std::string>& getTitle() = 0;
		/**
	 * The default Text for the title bar, when the above `optional` is `nullopt`
	 */
		virtual const std::string& getTitleDefault() = 0;
		/**
	 * The ID appended onto the title to be used by the ImGuiWindow, so it doesn't crash.
	 * This should never change!
	 */
		virtual const std::string& getWindowID() = 0;

		/**
	 * Get the name of this window in the arcdps options.
	 * This has to return a string, else ImGui will cause an assert failure.
	 * This is set in the options
	 */
		virtual std::optional<std::string>& getAppearAsInOption() = 0;

		/**
	 * Default for appear as in option, if the optional there is nullopt.
	 */
		virtual const std::string& getAppearAsInOptionDefault() = 0;

		/**
	 * This has to be `true` to show the background.
	 * Therefore setting it to `true` by default is recommended.
	 */
		virtual bool& getShowBackground() = 0;

		/**
	 * This padding is optional, if the optional is not set, no change to the window padding is applied.
	 * Default should be nullopt.
	 */
		virtual std::optional<ImVec2>& getPadding() = 0;

		/**
	 * Check if the window should have a maximal height.
	 * If this is true, the variable set with `SetMaxHeightCursorPos` will be used to determine the max-height.
	 * This has to be tracked by the implementation, cause this window has no idea what it's content looks like.
	 */
		virtual bool getMaxHeightActive() { return false; }

		/**
	 * The current language the plugin is set to.
	 * Default to english, this should be overridden to get proper language selection.
	 * The current language can be tracked with the `unofficial_extras` add-on.
	 */
		virtual gwlanguage getCurrentLanguage() { return GWL_ENG; }

		/**
	 * This can be called to draw the SubMenu of the "Style".
	 * It will only draw the submenu and not the Menu option itself.
	 *
	 * It is called by default at the bottom of the context menu.
	 * You can disable that by setting the flag `MainWindowFlags_NoPositionMenu`
	 */
		virtual void DrawStyleSettingsSubMenu();

		/**
	 * Override if you want to add additional checks when the keybind is pressed.
	 * e.g. arcdps is hidden
	 */
		virtual bool KeyBindPressed();

	private:
		bool mPaddingActive = false;
		float mPaddingBuffer[2]{};
		float mMaxHeightCursorPos = 0.f;
		std::string mAppearAsInOptionTextBuffer;
		std::string mTitleBuffer;

		std::vector<PreDrawHookFunction> mPreDrawHooks;
		std::vector<ContextMenuHookFunction> mContextMenuHooks;
		std::vector<DrawContentHookFunction> mDrawContentHooks;
		std::vector<InitHookFunction> mInitHooks;
		std::vector<DrawStyleSubMenuHookFunction> mDrawStyleSubMenuHooks;

		std::vector<std::unique_ptr<ComponentBase>> mComponents;
	};
} // namespace ArcdpsExtension
