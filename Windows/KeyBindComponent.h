#pragma once

#include "ComponentBase.h"
#include "../arcdps_structs_slim.h"

#include <ArcdpsUnofficialExtras/KeyBindStructs.h>
#include <cstddef>
#include <cstdint>

// Windows
#include <windows.h>
#include <winuser.h>

namespace ArcdpsExtension {
	class KeyBindComponent : public ComponentBase {
	public:
		explicit KeyBindComponent(MainWindow* pMainWindow);

	protected:
		/**
		 * Override if you want to add additional checks when the keybind is pressed.
		 * e.g. arcdps is hidden
		 */
		virtual bool getKeyBindSwitch() { return true; }
		/**
		 * This function does the actual changes to the MainWindow and is called after `getKeyBindSwitch` returned `true`.
		 * return `true` if the KeyPressEvent should be consumed.
		 *
		 * You don't need to override this, you want to use `getKeyBindSwitch` instead.
		 */
		virtual bool KeyBindPressed();

		/**
		 * The KeyBind this window should react on.
		 */
		virtual KeyBinds::Key& getKeyBind() = 0;

		/**
		 * Close this window with a single press of ESC.
		 * This function is checked everytime ESC is pressed and the window is open.
		 * Use this to add additional checks, e.g. if arcdps is is hidden and more.
		 * return `true` if closing should be done.
		 */
		virtual bool getCloseWithEsc() { return false; }
		/**
		 * This variable is only checked once, it should be `true`, if this feature is planned to be used.
		 * If this is `false`, the ESC key is not checked at all.
		 */
		virtual bool getCloseWithEscActive() { return false; }
		/**
		 * Contains the actual logic to hide the attached window.
		 * return `true` if the KeyPressEvent should be consumed.
		 *
		 * You don't need to override this, you want to use `getCloseWithEsc` instead.
		 */
		virtual bool EscPressed();

		/**
		 * Get the current KeyBoardLayout.
		 * Since we are not in the mainthread, this will most likely be wrong!
		 * This can be tracked in arcdps `mod_wnd`.
		 */
		virtual HKL getCurrentHKL() { return GetKeyboardLayout(NULL); }

		/**
		 * The current language the plugin is set to.
		 * Default to english, this should be overridden to get proper language selection.
		 * The current language can be tracked with the `unofficial_extras` add-on.
		 */
		virtual gwlanguage getCurrentLanguage() { return GWL_ENG; }

	private:
		uint64_t mKeyBindHandlerId = 0;
		uint64_t mKeyBindEscHandlerId = 0;
	};
} // namespace ArcdpsExtension
