#pragma once

#include "../../Singleton.h"
#include "../../arcdps_structs.h"
#include "../MainWindow.h"
#include "DemoTable.h"

#include <imgui/imgui.h>
#include <memory>
#include <optional>
#include <string>

namespace ArcdpsExtension {
	class DemoTableWindow final : public MainWindow, public Singleton<DemoTableWindow> {
	public:
		DemoTableWindow();
		bool& GetOpenVar() override;
		void SetMaxHeightCursorPos(float pNewCursorPos = ImGui::GetCursorPosY()) override;
		bool& GetShowScrollbar() override;

	protected:
		void DrawContextMenu() override;
		void DrawContent() override;

		std::string_view getTitleDefault() override;
		std::optional<std::string>& getTitle() override;
		std::string_view getWindowID() override;
		bool& getShowTitleBar() override;
		bool& getShowBackground() override;
		std::optional<ImVec2>& getPadding() override;
		SizingPolicy& getSizingPolicy() override;
		std::optional<std::string>& getAppearAsInOption() override;
		std::string_view getAppearAsInOptionDefault() override;

	private:
		bool mOpen = true;
		std::string mTitleDefault = "Demo Table Window";
		std::string mWindowID = "Demo Table Window";
		std::optional<std::string> mTitle;
		bool mShowTitleBar = true;
		bool mGetShowBackground = true;
		bool mShowScrollbar = true;
		std::optional<ImVec2> mPadding;
		SizingPolicy mSizingPolicy = SizingPolicy::SizeToContent;
		int mCurrentRow = 0;
		std::optional<std::string> mAppearAsInOptionOpt;
		const std::string mAppearAsInOptionDefault = "Demo Table Window";

		std::unique_ptr<DemoTable> mTable;
	};
} // namespace ArcdpsExtension
