#pragma once

#include "../KeyBindComponent.h"

namespace ArcdpsExtension {
	class DemoKeyBindComponent final : public KeyBindComponent {
	public:
		explicit DemoKeyBindComponent(MainWindow* pMainWindow)
			: KeyBindComponent(pMainWindow) {}

	protected:
		KeyBinds::Key& getKeyBind() override;
		bool getCloseWithEsc() override;
		bool getCloseWithEscActive() override { return true; };

	private:
		KeyBinds::Key mKeyBind;
	};
} // namespace ArcdpsExtension
