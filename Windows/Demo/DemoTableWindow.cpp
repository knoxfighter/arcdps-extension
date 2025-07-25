#include "DemoTableWindow.h"

#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
#include "DemoKeyBindComponent.h"
#endif // ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS

#include "DemoTable.h"
#include "DemoPositioningComponent.h"

ArcdpsExtension::DemoTableWindow::DemoTableWindow() : MainWindow() {
	CreateComponent<DemoPositioningComponent>();

#if ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS
	CreateComponent<DemoKeyBindComponent>();
#endif // ARCDPS_EXTENSION_UNOFFICIAL_EXTRAS

	mTable = std::make_unique<DemoTable>(this);
}

bool& ArcdpsExtension::DemoTableWindow::GetOpenVar() {
	return mOpen;
}

void ArcdpsExtension::DemoTableWindow::SetMaxHeightCursorPos(float pNewCursorPos) {
	MainWindow::SetMaxHeightCursorPos(pNewCursorPos - GImGui->Style.ItemSpacing.y);
}

void ArcdpsExtension::DemoTableWindow::DrawContextMenu() {
	ImGui::Text("testWindowContextMenu");

	mTable->DrawColumnSetupMenu();

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PopStyleVar();
}

void ArcdpsExtension::DemoTableWindow::DrawContent() {
	mTable->Draw();
}

const std::string& ArcdpsExtension::DemoTableWindow::getTitleDefault() {
	return mTitleDefault;
}

std::optional<std::string>& ArcdpsExtension::DemoTableWindow::getTitle() {
	return mTitle;
}

const std::string& ArcdpsExtension::DemoTableWindow::getWindowID() {
	return mWindowID;
}

bool& ArcdpsExtension::DemoTableWindow::getShowTitleBar() {
	return mShowTitleBar;
}

bool& ArcdpsExtension::DemoTableWindow::getShowBackground() {
	return mGetShowBackground;
}

bool& ArcdpsExtension::DemoTableWindow::GetShowScrollbar() {
	return mShowScrollbar;
}

std::optional<ImVec2>& ArcdpsExtension::DemoTableWindow::getPadding() {
	return mPadding;
}

SizingPolicy& ArcdpsExtension::DemoTableWindow::getSizingPolicy() {
	return mSizingPolicy;
}

std::optional<std::string>& ArcdpsExtension::DemoTableWindow::getAppearAsInOption() {
	return mAppearAsInOptionOpt;
}

const std::string& ArcdpsExtension::DemoTableWindow::getAppearAsInOptionDefault() {
	return mAppearAsInOptionDefault;
}
