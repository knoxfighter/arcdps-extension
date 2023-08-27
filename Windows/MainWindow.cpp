#include "MainWindow.h"

#include "../ExtensionTranslations.h"
#include "../imgui_stdlib.h"
#include "../Localization.h"
#include "../Widgets.h"

#include <magic_enum.hpp>

ArcdpsExtension::MainWindow::~MainWindow() {
	mComponents.clear();
}

void ArcdpsExtension::MainWindow::Draw(ImGuiWindowFlags imGuiWindowFlags, MainWindowFlags mainWindowFlags) {
	if (!GetOpenVar()) {
		return;
	}

	// general window options
	imGuiWindowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavFocus;

	if (!getShowTitleBar()) {
		imGuiWindowFlags |= ImGuiWindowFlags_NoTitleBar;
	}

	if (!getShowBackground()) {
		imGuiWindowFlags |= ImGuiWindowFlags_NoBackground;
	}

	if (!GetShowScrollbar()) {
		imGuiWindowFlags |= ImGuiWindowFlags_NoScrollbar;
	}

	const auto& padding = getPadding();
	bool paddingApplied = false;
	if (padding) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding.value());
		paddingApplied = true;
	}

	if (getSizingPolicy() == SizingPolicy::SizeToContent) {
		imGuiWindowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
	}

	if (getMaxHeightActive() && mThisWindow != nullptr && mMaxHeightCursorPos > 0) {
		ImVec2 windowPos = mThisWindow->Pos;

		if (!mThisWindow->ScrollbarY && static_cast<int32_t>(mMaxHeightCursorPos + windowPos.y /*- GImGui->Style.ItemSpacing.y*/) == static_cast<int32_t>(mThisWindow->ContentSize.y + mThisWindow->ContentRegionRect.Min.y)) {
			// [ DEBUG ]
			// ImVec2 max = mThisWindow->ContentSize;
			// ImVec2 min = mThisWindow->ContentRegionRect.Min;
			// max += min;
			// ImGui::GetForegroundDrawList(mThisWindow)->AddRect(min, max, 0xff00ff00);
		} else {
			// [ DEBUG ]
			// ImVec2 max = mThisWindow->ContentSize;
			// ImVec2 min = mThisWindow->ContentRegionRect.Min;
			// max += min;
			// ImVec2 cursorPos(max.x, mMaxHeightCursorPos + windowPos.y /*- GImGui->Style.ItemSpacing.y*/);
			// ImGui::GetForegroundDrawList(mThisWindow)->AddRect(min, max, 0xff0000ff);
			// ImGui::GetForegroundDrawList(mThisWindow)->AddRect(min, cursorPos, 0xffff0000);

			float minHeight = mThisWindow->InnerRect.Min.y + GImGui->Style.WindowPadding.y - windowPos.y + 5.f;
			float maxHeight = mMaxHeightCursorPos /*- GImGui->Style.ItemSpacing.y*/ + GImGui->Style.WindowPadding.y;
			ImGui::SetNextWindowSizeConstraints(ImVec2(50.f, minHeight), ImVec2(FLT_MAX, maxHeight));
		}
	}

	for (const auto& preDrawHook : mPreDrawHooks) {
		preDrawHook(imGuiWindowFlags);
	}

	const auto& titleOpt = getTitle();
	std::string title = titleOpt ? titleOpt.value() : getTitleDefault();
	title.append("###");
	title.append(getWindowID());
	ImGui::Begin(title.c_str(), &GetOpenVar(), imGuiWindowFlags);

	mThisWindow = ImGui::GetCurrentWindow();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
	if (ImGuiEx::BeginPopupContextWindow(nullptr, 1, ImGuiHoveredFlags_ChildWindows)) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);

		DrawContextMenu();

		for (const auto& contextMenuHook : mContextMenuHooks | std::ranges::views::reverse) {
			contextMenuHook();
		}

		if (!(mainWindowFlags & MainWindowFlags_NoStyleMenu)) {
			if (ImGui::BeginMenu(Localization::STranslate(ET_Style).c_str())) {
				DrawStyleSettingsSubMenu();

				for (const auto& drawStyleSubMenuHook : mDrawStyleSubMenuHooks | std::ranges::views::reverse) {
					drawStyleSubMenuHook();
				}

				ImGui::EndMenu();
			}
		}

		ImGui::PopItemFlag();
		ImGui::PopStyleVar();

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar();

	DrawContent();

	for (const auto& drawContentHook : mDrawContentHooks) {
		drawContentHook();
	}

	if (paddingApplied) {
		ImGui::PopStyleVar();
	}

	ImGui::End();
}

void ArcdpsExtension::MainWindow::Init() {
	for (const auto& initHook : mInitHooks) {
		initHook();
	}
}

void ArcdpsExtension::MainWindow::DrawOptionCheckbox() {
	const auto& appearAsInOptionOpt = getAppearAsInOption();
	const std::string& appearAsInOption = appearAsInOptionOpt ? appearAsInOptionOpt.value() : getAppearAsInOptionDefault();
	ImGui::Checkbox(appearAsInOption.c_str(), &GetOpenVar());
}

void ArcdpsExtension::MainWindow::SetMaxHeightCursorPos(float pNewCursorPos) {
	mMaxHeightCursorPos = pNewCursorPos;
}

ImGuiWindow* ArcdpsExtension::MainWindow::GetInnerWindow() {
	return mThisWindow;
}

void ArcdpsExtension::MainWindow::RegisterPreDrawHook(PreDrawHookFunction pFun) {
	mPreDrawHooks.emplace_back(pFun);
}

void ArcdpsExtension::MainWindow::RegisterContextMenuHook(ContextMenuHookFunction pFun) {
	mContextMenuHooks.emplace_back(pFun);
}

void ArcdpsExtension::MainWindow::RegisterContentHook(DrawContentHookFunction pFun) {
	mDrawContentHooks.emplace_back(pFun);
}

void ArcdpsExtension::MainWindow::RegisterInitHook(InitHookFunction pFun) {
	mInitHooks.emplace_back(pFun);
}

void ArcdpsExtension::MainWindow::RegisterDrawStyleSubMenuHook(DrawStyleSubMenuHookFunction pFun) {
	mDrawStyleSubMenuHooks.emplace_back(pFun);
}

float ArcdpsExtension::MainWindow::GetMaxCursorPos() {
	return mMaxHeightCursorPos;
}

void ArcdpsExtension::MainWindow::DrawStyleSettingsSubMenu() {
	ImGui::Checkbox(Localization::STranslate(ET_TitleBar).c_str(), &getShowTitleBar());
	ImGui::Checkbox(Localization::STranslate(ET_Background).c_str(), &getShowBackground());
	ImGui::Checkbox(Localization::STranslate(ET_Scrollbar).c_str(), &GetShowScrollbar());

	// padding
	ImGui::TextUnformatted(Localization::STranslate(ET_Padding).c_str());
	ImGui::SameLine();
	auto& padding = getPadding();
	mPaddingActive = padding.has_value();
	if (ImGui::Checkbox("##paddingCheckbox", &mPaddingActive)) {
		if (mPaddingActive == true) {
			padding = ImVec2();
		} else {
			padding.reset();
		}
	}
	ImGui::SameLine();
	if (!padding) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}
	mPaddingBuffer[0] = padding ? padding.value().x : 0.f;
	mPaddingBuffer[1] = padding ? padding.value().y : 0.f;
	if (ImGui::DragFloat2("##paddingInputFloat", mPaddingBuffer, 0.1f, 0, FLT_MAX, "%1.f")) {
		auto& value = padding.value();
		value.x = mPaddingBuffer[0] >= 0 ? mPaddingBuffer[0] : 0;
		value.y = mPaddingBuffer[1] >= 0 ? mPaddingBuffer[1] : 0;
	}
	if (!padding) {
		ImGui::PopItemFlag();
	}

	ImGui::Separator();

	// sizing policy
	ImGui::TextUnformatted(Localization::STranslate(ET_SizingPolicy).c_str());
	ImGui::SameLine();
	auto& sizingPolicy = getSizingPolicy();
	ImGuiEx::EnumCombo("##sizingPolicyEnumCombo", sizingPolicy, magic_enum::enum_values<SizingPolicy>());

	ImGui::Indent();
	DrawSizingPolicySubSettings(sizingPolicy);
	ImGui::Unindent();

	ImGui::Separator();

	auto& appearAsInOptionOpt = getAppearAsInOption();
	if (appearAsInOptionOpt) {
		mAppearAsInOptionTextBuffer = appearAsInOptionOpt.value();
	} else {
		mAppearAsInOptionTextBuffer = "";
	}
	if (ImGui::InputText(std::format("{}###appearAsInOption", Localization::STranslate(ET_AppearAsInOption)).c_str(), &mAppearAsInOptionTextBuffer)) {
		if (mAppearAsInOptionTextBuffer.empty()) {
			appearAsInOptionOpt.reset();
		} else {
			appearAsInOptionOpt = mAppearAsInOptionTextBuffer;
		}
	}

	auto& titleOpt = getTitle();
	if (titleOpt) {
		mTitleBuffer = titleOpt.value();
	} else {
		mTitleBuffer = "";
	}
	if (ImGui::InputText(std::format("{}###titleBar", Localization::STranslate(ET_TitleBarText)).c_str(), &mTitleBuffer)) {
		if (mTitleBuffer.empty()) {
			titleOpt.reset();
		} else {
			titleOpt = mTitleBuffer;
		}
	}
}

bool ArcdpsExtension::MainWindow::KeyBindPressed() {
	return true;
}
