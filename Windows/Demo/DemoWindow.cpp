#include "DemoWindow.h"

#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
#include "DemoKeyBindComponent.h"
#endif // ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
#include "DemoPositioningComponent.h"

ArcdpsExtension::DemoWindow::DemoWindow() : MainWindow() {
	CreateComponent<DemoPositioningComponent>();
#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
	CreateComponent<DemoKeyBindComponent>();
#endif // ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
}

bool& ArcdpsExtension::DemoWindow::GetOpenVar() {
	return mOpen;
}

void ArcdpsExtension::DemoWindow::SetMaxHeightCursorPos(float pNewCursorPos) {
	MainWindow::SetMaxHeightCursorPos(pNewCursorPos - GImGui->Style.ItemSpacing.y);
}

void ArcdpsExtension::DemoWindow::DrawContextMenu() {
	ImGui::Text("testWindowContextMenu");

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PopStyleVar();
}

void ArcdpsExtension::DemoWindow::DrawContent() {
	mCurrentRow = 0;
	ImGui::Text("testWindow");
	newRow();
	ImGui::Text("some additional text");
	newRow();
	ImGui::Text("more text :P");
	newRow();
}

void ArcdpsExtension::DemoWindow::newRow() {
	if (mCurrentRow < mMaxDisplayed) {
		SetMaxHeightCursorPos();
	}

	++mCurrentRow;
}

const std::string& ArcdpsExtension::DemoWindow::getTitleDefault() {
	return mTitleDefault;
}

std::optional<std::string>& ArcdpsExtension::DemoWindow::getTitle() {
	return mTitle;
}

const std::string& ArcdpsExtension::DemoWindow::getWindowID() {
	return mWindowID;
}

bool& ArcdpsExtension::DemoWindow::getShowTitleBar() {
	return mShowTitleBar;
}

bool& ArcdpsExtension::DemoWindow::getShowBackground() {
	return mGetShowBackground;
}

bool& ArcdpsExtension::DemoWindow::GetShowScrollbar() {
	return mShowScrollbar;
}

std::optional<ImVec2>& ArcdpsExtension::DemoWindow::getPadding() {
	return mPadding;
}

SizingPolicy& ArcdpsExtension::DemoWindow::getSizingPolicy() {
	return mSizingPolicy;
}

bool ArcdpsExtension::DemoWindow::getMaxHeightActive() {
	return mMaxDisplayed != 0;
}

std::optional<std::string>& ArcdpsExtension::DemoWindow::getAppearAsInOption() {
	return mAppearAsInOptionOpt;
}

const std::string& ArcdpsExtension::DemoWindow::getAppearAsInOptionDefault() {
	return mAppearAsInOptionDefault;
}

void ArcdpsExtension::DemoWindow::DrawStyleSettingsSubMenu() {
	MainWindow::DrawStyleSettingsSubMenu();

	ImGui::InputInt("max displayed", &mMaxDisplayed);
}
