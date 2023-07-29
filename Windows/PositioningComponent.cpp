#include "PositioningComponent.h"

#include "../ExtensionTranslations.h"
#include "../Localization.h"
#include "../Widgets.h"

#include <algorithm>

PositioningComponent::PositioningComponent(MainWindow* pMainWindow) : ComponentBase(pMainWindow) {
	pMainWindow->RegisterPreDrawHook([this](ImGuiWindowFlags& PH1) {
		PreDrawHookFunction(std::forward<decltype(PH1)>(PH1));
	});

	pMainWindow->RegisterContextMenuHook([this] {
		DrawPositionSettingsSubMenu();
	});

	PositioningComponentImGuiHook::POSITIONING_COMPONENTS.emplace_back(this);
}

void PositioningComponent::PreDrawHookFunction(ImGuiWindowFlags& pFlags) {
	if (getPositionMode() != Position::Manual) {
		pFlags |= ImGuiWindowFlags_NoMove;
	}
}

void PositioningComponent::DrawPositionSettingsSubMenu() {
	if (ImGui::BeginMenu(Localization::STranslate(ET_Position).c_str())) {
		Position& position = getPositionMode();
		CornerPosition& cornerPosition = getCornerPosition();
		ImVec2& windowVector = getCornerVector();
		CornerPosition& anchorPanelCornerPosition = getAnchorPanelCorner();
		CornerPosition& selfPanelCornerPosition = getSelfPanelCorner();
		ImGuiID& anchorWindowId = getFromWindowId();

		ImGuiEx::RadioButton(to_string(Position::Manual).c_str(), position, Position::Manual);

		ImGuiEx::RadioButton(to_string(Position::ScreenRelative).c_str(), position, Position::ScreenRelative);
		if (position == Position::ScreenRelative) {
			ImGui::Indent(15.f);

			ImGui::PushID("cornerPositionRadioButton");
			ImGuiEx::RadioButton(to_string(CornerPosition::TopLeft).c_str(), cornerPosition, CornerPosition::TopLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::TopRight).c_str(), cornerPosition, CornerPosition::TopRight);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomLeft).c_str(), cornerPosition, CornerPosition::BottomLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomRight).c_str(), cornerPosition, CornerPosition::BottomRight);
			ImGui::PopID();

			ImGui::PushItemWidth(80.f);
			ImGui::DragFloat("x", &windowVector.x);
			ImGui::DragFloat("y", &windowVector.y);
			ImGui::PopItemWidth();

			ImGui::Unindent(15.f);
		}

		ImGuiEx::RadioButton(to_string(Position::WindowRelative).c_str(), position, Position::WindowRelative);
		if (position == Position::WindowRelative) {
			ImGui::Indent(15.f);

			ImGui::TextUnformatted(Localization::STranslate(ET_FromAnchorPanelCorner).c_str());
			ImGui::PushID("anchorPanelCornerPositionRadioButton");
			ImGuiEx::RadioButton(to_string(CornerPosition::TopLeft).c_str(), anchorPanelCornerPosition, CornerPosition::TopLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::TopRight).c_str(), anchorPanelCornerPosition, CornerPosition::TopRight);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomLeft).c_str(), anchorPanelCornerPosition, CornerPosition::BottomLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomRight).c_str(), anchorPanelCornerPosition, CornerPosition::BottomRight);
			ImGui::PopID();

			ImGui::TextUnformatted(Localization::STranslate(ET_ThisPanelCorner).c_str());
			ImGui::PushID("selfPanelCornerPositionRadioButton");
			ImGuiEx::RadioButton(to_string(CornerPosition::TopLeft).c_str(), selfPanelCornerPosition, CornerPosition::TopLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::TopRight).c_str(), selfPanelCornerPosition, CornerPosition::TopRight);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomLeft).c_str(), selfPanelCornerPosition, CornerPosition::BottomLeft);
			ImGuiEx::RadioButton(to_string(CornerPosition::BottomRight).c_str(), selfPanelCornerPosition, CornerPosition::BottomRight);
			ImGui::PopID();

			ImGui::PushItemWidth(80.f);
			ImGui::DragFloat("x", &windowVector.x);
			ImGui::DragFloat("y", &windowVector.y);
			ImGui::PopItemWidth();

			ImGuiID shownWindowName = anchorWindowId;
			ImGuiWindow* selectedWindow = ImGui::FindWindowByID(shownWindowName);
			std::string selectedWindowName;
			if (selectedWindow) {
				selectedWindowName = selectedWindow->Name;
				const auto findRes = selectedWindowName.find('#');
				if (findRes != std::string::npos) {
					selectedWindowName = selectedWindowName.substr(0, findRes);
				}
			}

			if (ImGui::BeginCombo(Localization::STranslate(ET_AnchorWindow).c_str(), selectedWindowName.c_str()) == true) {
				// This doesn't return the same thing as RootWindow interestingly enough, RootWindow returns a "higher" parent
				ImGuiWindow* parent = ImGui::GetCurrentWindowRead();
				while (parent->ParentWindow != nullptr) {
					parent = parent->ParentWindow;
				}

				for (ImGuiWindow* window : ImGui::GetCurrentContext()->Windows) {
					if (window != mMainWindow->GetInnerWindow() && // Not the window this MainWindow represents
						window != parent &&                        // Not the window we're currently in
						window->ParentWindow == nullptr &&         // Not a child window of another window
						window->Hidden == false &&                 // Not hidden
						window->WasActive == true &&
						// Not closed (we check ->WasActive because ->Active might not be true yet if the window gets rendered after this one)
						window->IsFallbackWindow == false &&             // Not default window ("Debug##Default")
						(window->Flags & ImGuiWindowFlags_Tooltip) == 0) // Not a tooltip window ("##Tooltip_<id>")
					{
						std::string windowName = "(";
						windowName += std::to_string(
								PositioningComponentImGuiHook::ANCHORING_HIGHLIGHTED_WINDOWS.size()
						);
						windowName += ") ";
						windowName += window->Name;

						// Print the window with ##/### part still there - it doesn't really hurt (and with the presence
						// of ### it's arguably more correct, even though it probably doesn't matter for a Selectable if
						// the id is unique or not)
						if (ImGui::Selectable(windowName.c_str())) {
							anchorWindowId = window->ID;
						}

						PositioningComponentImGuiHook::ANCHORING_HIGHLIGHTED_WINDOWS.emplace_back(window->ID);
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Unindent();
		}

		ImGui::EndMenu();
	}
}

void PositioningComponent::Reposition() {
	ImGuiWindow* window = mMainWindow->GetInnerWindow();
	if (window != nullptr) {
		ImGuiEx::WindowReposition(
				window,
				getPositionMode(),
				getCornerVector(),
				getCornerPosition(),
				getFromWindowId(),
				getAnchorPanelCorner(),
				getSelfPanelCorner()
		);
	}
}

PositioningComponent::~PositioningComponent() {
	auto& components = PositioningComponentImGuiHook::POSITIONING_COMPONENTS;
	if (!components.empty()) {
		components.erase(std::ranges::remove(components, this).begin(), components.end());
	}
}

void PositioningComponentImGuiHook::InstallHooks(ImGuiContext* imGuiContext) {
	if (imGuiContext != nullptr) {
		ImGuiContextHook contextHook;
		contextHook.Type = ImGuiContextHookType_NewFramePost;
		contextHook.Callback = PostNewFrame;
		ImGui::AddContextHook(imGuiContext, &contextHook);

		contextHook.Type = ImGuiContextHookType_EndFramePost;
		contextHook.Callback = PreEndFrame;
		ImGui::AddContextHook(imGuiContext, &contextHook);
	}
}

void PositioningComponentImGuiHook::PostNewFrame(ImGuiContext* pImguiContext, ImGuiContextHook*) {
	for (auto* mainWindow : POSITIONING_COMPONENTS) {
		mainWindow->Reposition();
	}
}

ImVec2 CalcCenteredPosition(const ImVec2& pBoundsPosition, const ImVec2& pBoundsSize, const ImVec2& pItemSize) {
	return ImVec2{
			pBoundsPosition.x + (std::max)((pBoundsSize.x - pItemSize.x) * 0.5f, 0.0f),
			pBoundsPosition.y + (std::max)((pBoundsSize.y - pItemSize.y) * 0.5f, 0.0f)};
}

void PositioningComponentImGuiHook::PreEndFrame(ImGuiContext* pImguiContext, ImGuiContextHook*) {
	float font_size = pImguiContext->FontSize * 2.0f;

	for (size_t i = 0; i < ANCHORING_HIGHLIGHTED_WINDOWS.size(); i++) {
		ImGuiWindow* window = ImGui::FindWindowByID(ANCHORING_HIGHLIGHTED_WINDOWS[i]);

		if (window != nullptr) {
			std::string text = std::to_string(i);

			ImVec2 regionSize{32.0f, 32.0f};
			ImVec2 regionPos = CalcCenteredPosition(window->Pos, window->Size, regionSize);

			ImVec2 textSize = pImguiContext->Font->CalcTextSizeA(font_size, FLT_MAX, -1.0f, text.c_str());
			ImVec2 textPos = CalcCenteredPosition(regionPos, regionSize, textSize);

			window->DrawList->AddRectFilled(regionPos, regionPos + regionSize, IM_COL32(0, 0, 0, 255));
			window->DrawList->AddText(pImguiContext->Font, font_size, textPos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
		}
	}

	ANCHORING_HIGHLIGHTED_WINDOWS.clear();
}
