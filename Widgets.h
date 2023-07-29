#pragma once

#include "arcdps_structs.h"
#include "rapidfuzz_amalgamated.hpp"

#if __has_include(<imgui/imgui.h>)
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#else
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#endif


#include <algorithm>
#include <format>
#include <functional>
#include <map>
#include <ranges>

namespace ImGuiEx {
	bool Spinner(const char* label, float radius, float thickness, const ImU32& color);
	bool SpinnerAligned(const char* label, float radius, float thickness, const ImU32& color, Alignment alignment);
	void AlignedTextColumn(Alignment alignment, const char* text, ...);
	void TableHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment = Alignment::Left);
	void AlignedProgressBar(float fraction, const ImVec2& size_arg, const char* overlay, Alignment alignment);
	bool BeginMenu(const char* label, bool enabled, bool& hovered);
	void BeginMenuChild(const char* child_str_id, const char* menu_label, std::function<void()> draw_func);
	void BeginMenu(const char* menu_label, std::function<void()> draw_func);
	bool BeginPopupContextWindow(const char* str_id, ImGuiPopupFlags popup_flags, ImGuiHoveredFlags hovered_flags);
	void MenuItemTableColumnVisibility(ImGuiTable* table, int columnIdx);
	bool TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags, void* icon);

	/**
	 * Same as `ImGui::BeginTable()` with additional parameter `child_window_flags`
	 * That parameter are flags for the child window to allow further personalization.
	 * It is only used if the table uses a child window to render (ScrollX or ScrollY set).
	 */
	bool BeginTable(const char* str_id, int column, ImGuiTableFlags flags = 0, const ImVec2& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f, ImGuiWindowFlags child_window_flags = 0);

	/**
	 * Only call this function at the end of a table row.
	 * If done before, it will not return the rect of the full table, but only the rect from beginning to the end of the current column.
	 */
	ImRect TableGetCurrentRowRect();
	/**
	 * This calls `TableGetCurrentRowRect` therefore this should also has the same limitations.
	 */
	bool TableIsMouseHoveringCurrentRow();
	// returns true if the window moved
	bool WindowReposition(ImGuiWindow* window, Position position, const ImVec2& cornerVector, CornerPosition cornerPosition, ImGuiID fromWindowID, CornerPosition anchorPanelCornerPosition, CornerPosition selfPanelCornerPosition);

	/**
	 * @return if element was freshly selected.
	 */
	template<typename E>
	requires std::is_enum_v<E>
	bool Selectable(E& storage, E value) {
		const bool selected = ImGui::Selectable(to_string(value).c_str());
		if (selected) {
			storage = value;
		}
		return selected;
	}

	/**
	 * @return A new element was selected
	 */
	template<typename E, std::ranges::range R>
	requires(std::is_enum_v<E> && std::same_as<std::ranges::range_value_t<R>, E>)
	bool EnumCombo(const char* label, E& storage, const R& values, const std::map<E, std::function<const std::string&()>>& pPopupText = {}) {
		if (ImGui::BeginCombo(label, to_string(storage).c_str())) {
			bool selected = false;
			for (const E& val : values) {
				if (ImGuiEx::Selectable(storage, val)) {
					selected = true;
				}

				if (ImGui::IsItemHovered()) {
					if (const auto& iterator = pPopupText.find(val); iterator != pPopupText.end()) {
						const auto& second = iterator->second();
						ImGui::SetTooltip("%s", second.c_str());
					}
				}
			}

			ImGui::EndCombo();

			return selected;
		}

		return false;
	}

	template<typename E>
	requires(std::is_enum_v<E>)
	bool EnumCombo(const char* label, E& storage, const std::initializer_list<E>& values, const std::map<E, std::function<const std::string&()>>& pPopupText = {}) {
		if (ImGui::BeginCombo(label, to_string(storage).c_str())) {
			bool selected = false;
			for (const E& val : values) {
				if (ImGuiEx::Selectable(storage, val)) {
					selected = true;
				}

				if (ImGui::IsItemHovered()) {
					if (const auto& iterator = pPopupText.find(val); iterator != pPopupText.end()) {
						ImGui::SetTooltip("%s", iterator->second().c_str());
					}
				}
			}

			ImGui::EndCombo();

			return selected;
		}

		return false;
	}

	/**
	 * This is not really save and also iterates over values, that are not defined!
	 * Be really careful, when using this!
	 *
	 * @return A new element was selected
	 */
	template<typename E>
	requires std::is_enum_v<E>
	bool EnumCombo(const char* label, E& storage, E lastElement, const std::map<uint64_t, std::function<const std::string&()>>& pPopupText = {}) {
		if (ImGui::BeginCombo(label, to_string(storage).c_str())) {
			bool selected = false;
			for (uint64_t i = 0; i < static_cast<uint64_t>(lastElement); ++i) {
				if (ImGuiEx::Selectable(storage, static_cast<E>(i))) {
					selected = true;
				}

				if (ImGui::IsItemHovered()) {
					if (const auto& iterator = pPopupText.find(i); iterator != pPopupText.end()) {
						ImGui::SetTooltip("%s", iterator->second().c_str());
					}
				}
			}

			ImGui::EndCombo();

			return selected;
		}
		return false;
	}

	template<typename E>
	requires std::is_enum_v<E>
	bool EnumRadioButton(int& buttonStorage, E value) {
		return ImGui::RadioButton(to_string(value).c_str(), &buttonStorage, static_cast<int>(value));
	}

	template<typename E>
	requires std::is_enum_v<E>
	bool EnumRadioButton(int& buttonStorage, E value, E& storage) {
		bool res = EnumRadioButton(buttonStorage, value);
		if (res) {
			storage = value;
		}

		return res;
	}

	// FIXME: This would work nicely if it was a public template, e.g. 'template<T> RadioButton(const char* label, T* v, T v_button)', but I'm not sure how we would expose it..
	// I have fixed it, don't know what the problem is with this ... --knox
	template<typename T>
	bool RadioButton(const char* label, T& v, T v_button) {
		const bool pressed = ImGui::RadioButton(label, v == v_button);
		if (pressed)
			v = v_button;
		return pressed;
	}

	template<class... Args>
	void TextColored(const ImVec4& col, const std::string& fmt, Args&&... args) {
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextEx(std::vformat(fmt, std::make_format_args(args...)).c_str(), NULL, ImGuiTextFlags_NoWidthForLargeClippedText); // Skip formatting
		ImGui::PopStyleColor();
	}

#ifdef _WIN32
	/// <summary>
	/// KeyInput, has `label` on the left, a textInput in the middle and the used shortcut at the right.
	/// Only defined for Windows, cause `VkKeyScanA`, `MapVirtualKeyA` and `GetKeyNameTextA` are windows-api functions.
	///
	///	The `buffer` has to be initialized to the keyContainer value, so the number is shown in the beginning.
	/// </summary>
	/// <param name="label">text to the left of the textInput</param>
	/// <param name="id">id for the inputtext (has to be unique and start with '##')</param>
	/// <param name="buffer">textbuffer for the textInput</param>
	/// <param name="bufSize">the size of the textbuffer</param>
	/// <param name="keyContainer">int container for the real keyCode</param>
	void KeyInput(const char* label, const char* id, char* buffer, size_t bufSize, WPARAM& keyContainer, const char* notSetText);
#endif


	// This code is derived from an issue in the ImGui github repo: https://github.com/ocornut/imgui/issues/1658#issuecomment-886171438
	// Therefore the original code is licensed under the same license as ImGui (MIT)
	//
	// returns if the value was changed.
	// `pPopupOpen` returns if the popup is open
	template<std::ranges::viewable_range T, typename ValueType = std::ranges::views::all_t<T>>
	// template<std::ranges::common_range T, typename ValueType = std::ranges::range_value_t<T>>
	bool FilteredCombo(const char* pLabel, const T& pContainer, ValueType& pCurrent, bool* pPopupOpen = nullptr) {
		// this is breaking the overloaded to_string functions (in theory it should work, but it doesn't :( )
		// using std::to_string;

		ImGuiContext& g = *GImGui;

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		static char searchInputBuffer[256] = {0};

		std::string popupName = std::format("###FilteredCombo_popup_name_{}", pLabel);

		// Display items
		// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
		bool valueChanged = false;

		const float expectedWidth = ImGui::CalcItemWidth();
		bool isNewOpen = false;
		float frameHeight = ImGui::GetFrameHeight();
		ImVec2 size(frameHeight, frameHeight);
		ImVec2 CursorPos = window->DC.CursorPos;
		ImVec2 pos = CursorPos + ImVec2(expectedWidth - frameHeight, 0);
		const ImRect bb(pos, pos + size);

		float ButtonTextAlignX = g.Style.ButtonTextAlign.x;
		g.Style.ButtonTextAlign.x = 0;
		if (ImGui::Button(std::format("{}###FilteredCombo_button_label_{}", to_string(pCurrent), pLabel).c_str(), ImVec2(expectedWidth, 0))) {
			ImGui::OpenPopup(popupName.c_str());
			isNewOpen = true;
			memset(searchInputBuffer, 0, sizeof searchInputBuffer);
		}
		g.Style.ButtonTextAlign.x = ButtonTextAlignX;
		// bool hovered = ImGui::IsItemHovered();
		// bool active = ImGui::IsItemActivated();
		// bool pressed = ImGui::IsItemClicked();

		// Render
		//const ImU32 bg_col = GetColorU32((active && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		//RenderFrame(bb.Min, bb.Max, bg_col, true, g.Style.FrameRounding);
		const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
		ImGui::RenderArrow(window->DrawList, bb.Min + ImVec2(ImMax(0.0f, (size.x - g.FontSize) * 0.5f), ImMax(0.0f, (size.y - g.FontSize) * 0.5f)), textColor, ImGuiDir_Down);

		ImVec2 item_max = ImGui::GetItemRectMax();
		ImGui::SetNextWindowPos({CursorPos.x, item_max.y});
		ImGui::SetNextWindowSize({ImGui::GetItemRectSize().x, 0});

		if (ImGui::BeginPopup(popupName.c_str())) {
			using CacheVectorType = std::tuple<double, ValueType>;
			static std::vector<CacheVectorType> valueCache;

			if (pPopupOpen != nullptr) {
				*pPopupOpen = true;
			}

			ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4) ImColor(240, 240, 240, 255));
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4) ImColor(0, 0, 0, 255));
			ImGui::PushItemWidth(-FLT_MIN);

			// Filter input
			if (isNewOpen) {
				ImGui::SetKeyboardFocusHere();
				for (const auto& val : pContainer) {
					valueCache.emplace_back(1, val);
				}
			}
			if (ImGui::InputText("###FilteredCombo_InputText", searchInputBuffer, sizeof searchInputBuffer)) {
				// input changed, recalculate fuzzy values
				valueCache.clear();
				if (searchInputBuffer[0] == '\0') {
					for (const auto& val : pContainer) {
						valueCache.emplace_back(1, val);
					}
				} else {
					for (const auto& val : pContainer) {
						const auto& valStr = to_string(val);
						double ratio = rapidfuzz::fuzz::ratio(valStr, searchInputBuffer);
						if (ratio >= 0.5)
							valueCache.emplace_back(ratio, val);
					}

					std::ranges::sort(valueCache, [](const CacheVectorType& val1, const CacheVectorType& val2) {
						return std::get<0>(val1) > std::get<0>(val2);
					});
				}
			}

			// Search Icon, you can use it if you load IconsFontAwesome5 https://github.com/juliettef/IconFontCppHeaders
			//const ImVec2 label_size = CalcTextSize(ICON_FA_SEARCH, NULL, true);
			//const ImVec2 search_icon_pos(ImGui::GetItemRectMax().x - label_size.x - style.ItemInnerSpacing.x * 2, window->DC.CursorPos.y + style.FramePadding.y + g.FontSize * 0.1f);
			//RenderText(search_icon_pos, ICON_FA_SEARCH);

			ImGui::PopStyleColor(2);

			if (ImGui::ListBoxHeader("###FilteredCombo_ItemList")) {
				for (const auto& value : valueCache) {
					const auto& val = std::get<1>(value);
					const bool itemSelected = (val == pCurrent);
					if (ImGui::Selectable(to_string(val).c_str(), itemSelected)) {
						valueChanged = true;
						pCurrent = val;
						ImGui::CloseCurrentPopup();
					}
					if (itemSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::ListBoxFooter();
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}


		if (valueChanged)
			ImGui::MarkItemEdited(g.CurrentWindow->DC.LastItemId);

		return valueChanged;
	}

	// template<typename T, typename ValueType>
	// requires std::ranges::common_range<T> && (!std::ranges::viewable_range<T>)
	// bool test(const char* pLabel, const T& pContainer, ValueType& pCurrent, bool* pPopupOpen = nullptr) {
	//     return FilteredCombo(pLabel, std::ranges::views::all(pContainer), pCurrent, pPopupOpen);
	// }
	template<std::ranges::common_range T, typename ValueType = std::ranges::range_value_t<T>>
	requires(!std::ranges::viewable_range<T>)
	bool FilteredCombo(const char* pLabel, const T& pContainer, ValueType& pCurrent, bool* pPopupOpen = nullptr) {
		return FilteredCombo(pLabel, std::ranges::views::all(pContainer), pCurrent, pPopupOpen);
	}
} // namespace ImGuiEx
