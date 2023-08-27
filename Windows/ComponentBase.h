#pragma once

namespace ArcdpsExtension {
	class MainWindow;

	class ComponentBase {
	public:
		explicit ComponentBase(MainWindow* pMainWindow) : mMainWindow(pMainWindow) {}
		virtual ~ComponentBase() {
			mMainWindow = nullptr;
		}

		// delete copy/move
		// ComponentBase(const ComponentBase& pOther) = default;
		// ComponentBase(ComponentBase&& pOther) noexcept = default;
		// ComponentBase& operator=(const ComponentBase& pOther) = default;
		// ComponentBase& operator=(ComponentBase&& pOther) noexcept = default;

	protected:
		MainWindow* mMainWindow;
	};
} // namespace ArcdpsExtension
