#pragma once

#include "../../Singleton.h"
#include "../MainWindow.h"
#include "DemoTable.h"

class DemoTableWindow final : public MainWindow, public Singleton<DemoTableWindow> {
public:
	DemoTableWindow();
	bool& GetOpenVar() override;
	void SetMaxHeightCursorPos(float pNewCursorPos = ImGui::GetCursorPosY()) override;

protected:
	void DrawContextMenu() override;
	void DrawContent() override;

	const std::string& getTitleDefault() override;
	std::optional<std::string>& getTitle() override;
	const std::string& getWindowID() override;
	bool& getShowTitleBar() override;
	bool& getShowBackground() override;
	std::optional<ImVec2>& getPadding() override;
	SizingPolicy& getSizingPolicy() override;
	std::optional<std::string>& getAppearAsInOption() override;
	const std::string& getAppearAsInOptionDefault() override;

public:
	bool& GetShowScrollbar() override;

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
