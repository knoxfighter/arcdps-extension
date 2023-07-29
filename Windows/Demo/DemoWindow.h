#pragma once

#include "../../Singleton.h"
#include "../MainWindow.h"

class DemoWindow final : public MainWindow, public Singleton<DemoWindow> {
public:
	DemoWindow();
	bool& GetOpenVar() override;
	void SetMaxHeightCursorPos(float pNewCursorPos = ImGui::GetCursorPosY()) override;

protected:
	void DrawContextMenu() override;
	void DrawContent() override;

private:
	bool mOpen = true;
	std::string mTitleDefault = "Demo Window";
	std::string mWindowID = "Demo Window";
	std::optional<std::string> mTitle;
	bool mShowTitleBar = true;
	bool mGetShowBackground = true;
	bool mShowScrollbar = true;
	std::optional<ImVec2> mPadding;
	SizingPolicy mSizingPolicy = SizingPolicy::SizeToContent;
	int mMaxDisplayed = 0;
	int mCurrentRow = 0;
	std::optional<std::string> mAppearAsInOptionOpt;
	const std::string mAppearAsInOptionDefault = "Demo Window";

	void newRow();

protected:
	const std::string& getTitleDefault() override;
	std::optional<std::string>& getTitle() override;
	const std::string& getWindowID() override;
	bool& getShowTitleBar() override;
	bool& getShowBackground() override;
	std::optional<ImVec2>& getPadding() override;
	SizingPolicy& getSizingPolicy() override;
	bool getMaxHeightActive() override;
	std::optional<std::string>& getAppearAsInOption() override;
	const std::string& getAppearAsInOptionDefault() override;
	void DrawStyleSettingsSubMenu() override;

public:
	bool& GetShowScrollbar() override;
};
