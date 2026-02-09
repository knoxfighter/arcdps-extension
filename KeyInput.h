#pragma once

#include <ArcdpsUnofficialExtras/KeyBindStructs.h>
#include <ArcdpsUnofficialExtras/Definitions.h>
#include <cstdint>
#include <windows.h>

/**
 * How to use:
 *
 * Call `KeyCodeInputWndHandle` in `mod_wnd` or your specific window handler.
 * Call `KeyCodeInput` whenever you want to have a KeyBind selection.
 * the wndHandler only has to be called once.
 */
namespace ImGuiEx {
	enum KeyCodeInputFlags_ : int32_t {
		KeyCodeInputFlags_NoModifier = 1 << 0,
		KeyCodeInputFlags_NoMouse = 1 << 1,
		KeyCodeInputFlags_OnlyGW2Keys = 1 << 2,
		KeyCodeInputFlags_FixedModifier = 1 << 3, // Will set NoModifier internally, Always use the Modifier given in parameter `pFixedModifier`.
	};
	typedef int32_t KeyCodeInputFlags; // enum KeyCodeInputFlags_

	/**
	 * \brief Has to be called in `mod_wnd` or your specific window handler. This function only has to be called once per program.
	 * \param pWindowHandle WndHandle of mod_wnd
	 * \param pMessage Message of mod_wnd
	 * \param pAdditionalW wParam of mod_wnd
	 * \param pAdditionalL lParam of mod_wnd
	 * \return `true` if the event was consumed. Make sure to stop the event calls to run further.
	 */
	bool KeyCodeInputWndHandle(HWND pWindowHandle, UINT pMessage, WPARAM pAdditionalW, LPARAM pAdditionalL);

	/**
	 * \brief Show a keybind on a button. On pressing that button a popup is opened and all keyevents are used to fill the keybind. (Kind of like GW2 does it).
	 * \param pLabel Shown as text next to the input button. Also shown in the popup header. Will be used as ID for the button.
	 * \param pKeyContainer The container of the original key, when pressing `Apply` it will be saved in there.
	 * \param pLanguage The language in which this Tool should be shown.
	 * \param pHkl current Keyboardlayout, has to be tracked, cause `GetKeyboardLayout` is thread dependent. You can get it by tracking window events or by calling `GetKeyboardLayout` with the correct threadId as param.
	 * \param pFlags Additional flags for the input.
	 */
	bool KeyCodeInput(const char* pLabel, KeyBinds::Key& pKeyContainer, Language pLanguage, HKL pHkl, KeyCodeInputFlags pFlags = 0, KeyBinds::Modifier pFixedModifier = 0);

	void OpenKeyCodePopupState(const KeyBinds::Key& pKeyContainer, KeyCodeInputFlags pFlags, KeyBinds::Modifier pFixedModifier = 0);
	void CloseKeyCodePopupState();
	void RestartKeyCodePopupState();
	void KeyCodeInputActiveFrame();
	const KeyBinds::Key& GetKeyCodeInputKeyState();
} // namespace ImGuiEx
