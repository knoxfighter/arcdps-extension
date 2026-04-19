#include "Widgets.h"

#include "ImGui_Math.h"

#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include <imgui/imgui_internal.h>
#include <cstdarg>

// Windows
#include <wtypes.h>

#pragma warning(push)
// disable warning for implicit conversion (for now)
#pragma warning(disable : 4244)

static const float TABLE_BORDER_SIZE = 1.0f; // hardcoded in imgui 1.80 as well

using namespace ImGui;

namespace ImGuiEx {
	bool Spinner(const char* label, float radius, float thickness, const ImU32& color) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size((radius) *2, (radius + style.FramePadding.y) * 2);

		const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		ImGui::ItemSize(bb, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		// Render
		window->DrawList->PathClear();

		int num_segments = 15;
		int start = abs(ImSin(g.Time * 1.8) * (num_segments - 5));

		const float a_min = IM_PI * 2.0f * ((float) start) / (float) num_segments;
		const float a_max = IM_PI * 2.0f * ((float) num_segments - 3) / (float) num_segments;

		const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

		for (int i = 0; i < num_segments; i++) {
			const float a = a_min + ((float) i / (float) num_segments) * (a_max - a_min);
			window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius, centre.y + ImSin(a + g.Time * 8) * radius));
		}

		window->DrawList->PathStroke(color, false, thickness);

		return true;
	}

	bool SpinnerAligned(const char* label, float radius, float thickness, const ImU32& color, Alignment alignment) {
		const float posX = ImGui::GetCursorPosX();
		float newX = posX;
		float elementWidth = radius * 2 + thickness * 2;
		float columnWidth = ImGui::GetColumnWidth();

		switch (alignment) {
			case Alignment::Left:
				break;
			case Alignment::Center:
				newX = posX + columnWidth / 2 - elementWidth / 2;
				break;
			case Alignment::Right:
				newX = posX + columnWidth - elementWidth;
				break;
			// nothing to do when unaligned
			case Alignment::Unaligned: break;
		}

		// Clip to left, if text is bigger than current column
		if (newX < posX) {
			newX = posX;
		}

		ImGui::SetCursorPosX(newX);

		return Spinner(label, radius, thickness, color);
	}

	void AlignedTextColumn(Alignment alignment, const char* text, ...) {
		va_list args;
		va_start(args, text);
		char buf[4096];
		ImFormatStringV(buf, 4096, text, args);
		va_end(args);

		const float posX = ImGui::GetCursorPosX();
		float newX = posX;
		float textWidth = ImGui::CalcTextSize(buf).x;
		float columnWidth = ImGui::GetColumnWidth();

		switch (alignment) {
			case Alignment::Left:
				break;
			case Alignment::Center:
				newX = posX + columnWidth / 2 - textWidth / 2;
				break;
			case Alignment::Right:
				newX = posX + columnWidth - textWidth;
				break;
			// nothing to do when unaligned
			case Alignment::Unaligned: break;
		}

		// Clip to left, if text is bigger than current column
		if (newX < posX) {
			newX = posX;
		}

		ImGui::SetCursorPosX(newX);

		ImGui::TextUnformatted(buf);
	}

	// This is a copy of `ImGui::TableHeader(const char* label)`
	// I removed the line, where the header is printed, so i can use it with image only headers.
	// When "show_label" is true, the label will be printed, as in the default one.
	//
	// Emit a column header (text + optional sort order)
	// We cpu-clip text here so that all columns headers can be merged into a same draw call.
	// Note that because of how we cpu-clip and display sorting indicators, you _cannot_ use SameLine() after a TableHeader()
	void TableHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment) {
		// TODO change eventually
		const float image_size = 16.f;

		// Show label if texture is null
		if (!texture) {
			show_label = true;
		}

		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return;

		ImGuiTable* table = g.CurrentTable;
		IM_ASSERT_USER_ERROR_RET(table != NULL, "Call should only be done while in BeginTable() scope!");
		IM_ASSERT(table->CurrentColumn != -1);
		const int column_n = table->CurrentColumn;
		ImGuiTableColumn* column = &table->Columns[column_n];

		// Label
		if (label == NULL)
			label = "";
		const char* label_end = FindRenderedTextEnd(label);
		ImVec2 label_size = CalcTextSize(label, label_end, true);
		ImVec2 label_pos = window->DC.CursorPos;

		// If we already got a row height, there's use that.
		// FIXME-TABLE: Padding problem if the correct outer-padding CellBgRect strays off our ClipRect?
		ImRect cell_r = TableGetCellBgRect(table, column_n);
		float label_height = table->RowMinHeight - table->RowCellPaddingY * 2.0f;
		if (show_label) {
			label_height = ImMax(label_size.y, label_height);
		} else {
			label_height = ImMax(image_size, label_height);
		}

		// Calculate ideal size for sort order arrow
		float w_arrow = 0.0f;
		float w_sort_text = 0.0f;
		bool sort_arrow = false;
		char sort_order_suf[4] = "";
		const float ARROW_SCALE = 0.65f;
		if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
			w_arrow = ImTrunc(g.FontSize * ARROW_SCALE + g.Style.FramePadding.x);
			if (column->SortOrder != -1)
				sort_arrow = true;
			if (column->SortOrder > 0) {
				ImFormatString(sort_order_suf, IM_COUNTOF(sort_order_suf), "%d", column->SortOrder + 1);
				w_sort_text = g.Style.ItemInnerSpacing.x + CalcTextSize(sort_order_suf).x;
			}
		}

		// We feed our unclipped width to the column without writing on CursorMaxPos, so that column is still considered for merging.
		float max_pos_x = label_pos.x + w_sort_text + w_arrow;
		if (show_label) {
			max_pos_x += label_size.x;
		} else {
			max_pos_x += image_size;
		}
		column->ContentMaxXHeadersUsed = ImMax(column->ContentMaxXHeadersUsed, sort_arrow ? cell_r.Max.x : ImMin(max_pos_x, cell_r.Max.x));
		column->ContentMaxXHeadersIdeal = ImMax(column->ContentMaxXHeadersIdeal, max_pos_x);

		// Keep header highlighted when context menu is open.
		ImGuiID id = window->GetID(label);
		ImRect bb(cell_r.Min.x, cell_r.Min.y, cell_r.Max.x, ImMax(cell_r.Max.y, cell_r.Min.y + label_height + g.Style.CellPadding.y * 2.0f));
		ItemSize(ImVec2(0.0f, label_height)); // Don't declare unclipped width, it'll be fed ContentMaxPosHeadersIdeal
		if (!ItemAdd(bb, id))
			return;

		//GetForegroundDrawList()->AddRect(cell_r.Min, cell_r.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]
		//GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]

		// Using AllowOverlap mode because we cover the whole cell, and we want user to be able to submit subsequent items.
		const bool highlight = (table->HighlightColumnHeader == column_n);
		bool hovered, held;
		bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_AllowOverlap);
		if (held || hovered || highlight) {
			const ImU32 col = GetColorU32(held ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																				 : ImGuiCol_Header);
			//RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
			TableSetBgColor(ImGuiTableBgTarget_CellBg, col, table->CurrentColumn);
		} else {
			// Submit single cell bg color in the case we didn't submit a full header row
			if ((table->RowFlags & ImGuiTableRowFlags_Headers) == 0)
				TableSetBgColor(ImGuiTableBgTarget_CellBg, GetColorU32(ImGuiCol_TableHeaderBg), table->CurrentColumn);
		}
		RenderNavCursor(bb, id, ImGuiNavRenderCursorFlags_Compact | ImGuiNavRenderCursorFlags_NoRounding);
		if (held)
			table->HeldHeaderColumn = (ImGuiTableColumnIdx) column_n;
		window->DC.CursorPos.y -= g.Style.ItemSpacing.y * 0.5f;

		// Drag and drop to re-order columns.
		// FIXME-TABLE: Scroll request while reordering a column and it lands out of the scrolling zone.
		if (held && (table->Flags & ImGuiTableFlags_Reorderable) && IsMouseDragging(0) && !g.DragDropActive) {
			// - While moving a column it will jump on the other side of the mouse, so we also test for MouseDelta.x
			// - We need to handle reordering across hidden columns.
			//   In the configuration below, moving C to the right of E will lead to:
			//      ... C [D] E  --->  ... [D] E  C   (Column name/index)
			//      ... 2  3  4        ...  2  3  4   (Display order)
			// - The other constraints are enforced by TableQueueSetColumnDisplayOrder() which might early out.
			table->InstanceInteracted = table->InstanceCurrent;
			if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < cell_r.Min.x)
				if (ImGuiTableColumn* prev_column = (column->PrevEnabledColumn != -1) ? &table->Columns[column->PrevEnabledColumn] : NULL)
					TableQueueSetColumnDisplayOrder(table, column_n, prev_column->DisplayOrder);
			if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > cell_r.Max.x)
				if (ImGuiTableColumn* next_column = (column->NextEnabledColumn != -1) ? &table->Columns[column->NextEnabledColumn] : NULL)
					TableQueueSetColumnDisplayOrder(table, column_n, next_column->DisplayOrder);
		}

		// Sort order arrow
		const float ellipsis_max = ImMax(cell_r.Max.x - w_arrow - w_sort_text, label_pos.x);
		if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
			if (column->SortOrder != -1) {
				float x = ImMax(cell_r.Min.x, cell_r.Max.x - w_arrow - w_sort_text);
				float y = label_pos.y;
				if (column->SortOrder > 0) {
					PushStyleColor(ImGuiCol_Text, GetColorU32(ImGuiCol_Text, 0.70f));
					RenderText(ImVec2(x + g.Style.ItemInnerSpacing.x, y), sort_order_suf);
					PopStyleColor();
					x += w_sort_text;
				}
				RenderArrow(window->DrawList, ImVec2(x, y), GetColorU32(ImGuiCol_Text), column->SortDirection == ImGuiSortDirection_Ascending ? ImGuiDir_Up : ImGuiDir_Down, ARROW_SCALE);
			}

			// Handle clicking on column header to adjust Sort Order
			if (pressed && table->ReorderColumn != column_n) {
				ImGuiSortDirection sort_direction = TableGetColumnNextSortDirection(column);
				TableSetColumnSortDirection(column_n, sort_direction, g.IO.KeyShift);
			}
		}

		// Render clipped label. Clipping here ensure that in the majority of situations, all our header cells will
		// be merged into a single draw call.
		//window->DrawList->AddCircleFilled(ImVec2(ellipsis_max, label_pos.y), 40, IM_COL32_WHITE);
		if (show_label) {
			// RenderTextEllipsis(window->DrawList, label_pos, ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max,
			// 	ellipsis_max, label, label_end, &label_size);

			float newX = label_pos.x;

			switch (alignment) {
				case Alignment::Center:
					// newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (label_size.x / 2);
					newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (label_size.x / 2);
					// SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
					break;
				case Alignment::Right:
					newX = ellipsis_max - label_size.x;
					// SetCursorPosX(cursorPosX + textSpace - contentSize.x);
					break;
				// nothing to do
				case Alignment::Left:
				case Alignment::Unaligned: break;
			}

			RenderTextEllipsis(window->DrawList, ImVec2(newX, label_pos.y), ImVec2(ellipsis_max, bb.Max.y), ellipsis_max, label, label_end, &label_size);
		} else {
			float newX = label_pos.x;

			switch (alignment) {
				case Alignment::Center:
					// newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (image_size / 2);
					newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (image_size / 2);
					// SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
					break;
				case Alignment::Right:
					newX = ellipsis_max - image_size;
					// SetCursorPosX(cursorPosX + textSpace - contentSize.x);
					break;
				// nothing to do
				case Alignment::Left:
				case Alignment::Unaligned: break;
			}

			ImRect ibb(ImVec2(newX, label_pos.y), ImVec2(newX + image_size, label_pos.y + image_size));

			window->DrawList->AddImage(texture, ibb.Min, ibb.Max);
		}

		//const bool text_clipped = label_size.x > (ellipsis_max - label_pos.x);
		// if (text_clipped && hovered && g.ActiveId == 0)
		// SetItemTooltip("%.*s", (int) (label_end - label), label);
		if (IsItemHovered()) {
			SetTooltip("%s", label);
		}		

		// We don't use BeginPopupContextItem() because we want the popup to stay up even after the column is hidden
		// if (IsPopupOpenRequestForItem(ImGuiPopupFlags_None, id))
		// TableOpenContextMenu(column_n);
	}

	// This code can be used to make the text over the progressBar aligned.
	// This also uses imgui internals, which are likely to change between versions.
	void AlignedProgressBar(float fraction, const ImVec2& size_arg, const char* overlay, Alignment alignment) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
		ImRect bb(pos, pos + size);
		ItemSize(size, style.FramePadding.y);
		if (!ItemAdd(bb, 0))
			return;

		// Fraction < 0.0f will display an indeterminate progress bar animation
		// The value must be animated along with time, so e.g. passing '-1.0f * ImGui::GetTime()' as fraction works.
		const bool is_indeterminate = (fraction < 0.0f);
		if (!is_indeterminate)
			fraction = ImSaturate(fraction);

		// Out of courtesy we accept a NaN fraction without crashing
		float fill_n0 = 0.0f;
		float fill_n1 = (fraction == fraction) ? fraction : 0.0f;

		if (is_indeterminate) {
			const float fill_width_n = 0.2f;
			fill_n0 = ImFmod(-fraction, 1.0f) * (1.0f + fill_width_n) - fill_width_n;
			fill_n1 = ImSaturate(fill_n0 + fill_width_n);
			fill_n0 = ImSaturate(fill_n0);
		}

		// Render
		RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
		bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
		float fill_x0 = ImLerp(bb.Min.x, bb.Max.x, fill_n0);
		float fill_x1 = ImLerp(bb.Min.x, bb.Max.x, fill_n1);
		if (fill_x0 < fill_x1)
			RenderRectFilledInRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), fill_x0, fill_x1, style.FrameRounding);

		// Default displaying the fraction as percentage string, but user can override it
		// Don't display text for indeterminate bars by default
		char overlay_buf[32];
		if (!is_indeterminate || overlay != NULL) {
			if (!overlay) {
				ImFormatString(overlay_buf, IM_COUNTOF(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
				overlay = overlay_buf;
			}

			ImVec2 overlay_size = CalcTextSize(overlay, NULL);
			if (overlay_size.x > 0.0f) {
				ImVec2 alignRender = ImVec2(0.0f, 0.5f);
				switch (alignment) {
					case Alignment::Left:
						alignRender = ImVec2(0.f, 0.f);
						break;
					case Alignment::Center:
						alignRender = ImVec2(0.5f, 0.5f);
						break;
					case Alignment::Right:
						alignRender = ImVec2(1.f, 0.f);
						break;
				}

				float text_x = is_indeterminate ? (bb.Min.x + bb.Max.x - overlay_size.x) * 0.5f : fill_x1 + style.ItemSpacing.x;
				RenderTextClipped(
						ImVec2(ImClamp(text_x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max,
						overlay, NULL, &overlay_size, alignRender, &bb
				);
			}
		}
	}

	/**
	 * Example usage:
	 * ```
	    // for child windows
		ImGui::BeginChild("boonTableSettings", ImVec2(0, ImGui::GetTextLineHeight()));
		bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows);
		if (ImGui::BeginMenu("##boon-table-settings")) {
		    if (!hovered) {
		        ImGui::CloseCurrentPopup();
		    }
		    settingsUi.Draw();
		    ImGui::EndMenu();
		}
		ImGui::EndChild();

		// for menus without child
		bool hovered;
		if (ImGuiEx::BeginMenu("boon-table-settings", true, hovered)) {
		    settingsUi.Draw();

		    if (!(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows) || hovered) && !ImGui::IsAnyMouseDown()) {
		        ImGui::CloseCurrentPopup();
		    }

		    ImGui::EndMenu();
		}
		```
	 */

	static bool IsRootOfOpenMenuSet()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if ((g.OpenPopupStack.Size <= g.BeginPopupStack.Size) || (window->Flags & ImGuiWindowFlags_ChildMenu))
			return false;

		// Initially we used 'upper_popup->OpenParentId == window->IDStack.back()' to differentiate multiple menu sets from each others
		// (e.g. inside menu bar vs loose menu items) based on parent ID.
		// This would however prevent the use of e.g. PushID() user code submitting menus.
		// Previously this worked between popup and a first child menu because the first child menu always had the _ChildWindow flag,
		// making hovering on parent popup possible while first child menu was focused - but this was generally a bug with other side effects.
		// Instead we don't treat Popup specifically (in order to consistently support menu features in them), maybe the first child menu of a Popup
		// doesn't have the _ChildWindow flag, and we rely on this IsRootOfOpenMenuSet() check to allow hovering between root window/popup and first child menu.
		// In the end, lack of ID check made it so we could no longer differentiate between separate menu sets. To compensate for that, we at least check parent window nav layer.
		// This fixes the most common case of menu opening on hover when moving between window content and menu bar. Multiple different menu sets in same nav layer would still
		// open on hover, but that should be a lesser problem, because if such menus are close in proximity in window content then it won't feel weird and if they are far apart
		// it likely won't be a problem anyone runs into.
		const ImGuiPopupData* upper_popup = &g.OpenPopupStack[g.BeginPopupStack.Size];
		if (window->DC.NavLayerCurrent != upper_popup->ParentNavLayer)
			return false;
		return upper_popup->Window && (upper_popup->Window->Flags & ImGuiWindowFlags_ChildMenu) && IsWindowChildOf(upper_popup->Window, window, true);
	}

	bool BeginMenuEx(const char* label, const char* icon, bool enabled, bool& hoveredPar)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		bool menu_is_open = IsPopupOpen(id, ImGuiPopupFlags_None);

		// Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
		// The first menu in a hierarchy isn't so hovering doesn't get across (otherwise e.g. resizing borders with ImGuiButtonFlags_FlattenChildren would react), but top-most BeginMenu() will bypass that limitation.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
		if (window->Flags & ImGuiWindowFlags_ChildMenu)
			window_flags |= ImGuiWindowFlags_ChildWindow;

		// If a menu with same the ID was already submitted, we will append to it, matching the behavior of Begin().
		// We are relying on a O(N) search - so O(N log N) over the frame - which seems like the most efficient for the expected small amount of BeginMenu() calls per frame.
		// If somehow this is ever becoming a problem we can switch to use e.g. ImGuiStorage mapping key to last frame used.
		if (g.MenusIdSubmittedThisFrame.contains(id)) {
			if (menu_is_open)
				menu_is_open = BeginPopupMenuEx(id, label, window_flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
			else
				g.NextWindowData.ClearFlags(); // we behave like Begin() and need to consume those values
			return menu_is_open;
		}

		// Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
		g.MenusIdSubmittedThisFrame.push_back(id);

		ImVec2 label_size = CalcTextSize(label, NULL, true);

		// Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent without always being a Child window)
		// This is only done for items for the menu set and not the full parent window.
		const bool menuset_is_open = IsRootOfOpenMenuSet();
		if (menuset_is_open)
			PushItemFlag(ImGuiItemFlags_NoWindowHoverableCheck, true);

		// The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
		// However the final position is going to be different! It is chosen by FindBestWindowPosForPopup().
		// e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
		ImVec2 popup_pos;
		ImVec2 pos = window->DC.CursorPos;
		PushID(label);
		if (!enabled)
			BeginDisabled();

		bool pressed;

		// We use ImGuiSelectableFlags_NoSetKeyOwner to allow down on one menu item, move, up on another.
		const ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_NoSetKeyOwner | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_NoAutoClosePopups;
		ImGuiMenuColumns* offsets = &window->DC.MenuColumns;
		if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
			// Menu inside a horizontal menu bar
			// Selectable extend their highlight by half ItemSpacing in each direction.
			// For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
			window->DC.CursorPos.x += IM_TRUNC(style.ItemSpacing.x * 0.5f);
			PushStyleVarX(ImGuiStyleVar_ItemSpacing, style.ItemSpacing.x * 2.0f);
			ImVec2 text_pos(window->DC.CursorPos.x + offsets->OffsetLabel, pos.y + window->DC.CurrLineTextBaseOffset);
			pressed = ImGui::Selectable("", menu_is_open, selectable_flags, label_size);
			LogSetNextTextDecoration("[", "]");
			RenderText(text_pos, label);
			PopStyleVar();
			window->DC.CursorPos.x += IM_TRUNC(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
			popup_pos = ImVec2(pos.x - 1.0f - IM_TRUNC(style.ItemSpacing.x * 0.5f), text_pos.y - style.FramePadding.y + window->MenuBarHeight);
		} else {
			// Menu inside a regular/vertical menu
			// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
			//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.)
			float icon_w = (icon && icon[0]) ? CalcTextSize(icon, NULL).x : 0.0f;
			float checkmark_w = IM_TRUNC(g.FontSize * 1.20f);
			float min_w = offsets->DeclColumns(icon_w, label_size.x, 0.0f, checkmark_w); // Feedback to next frame
			float extra_w = ImMax(0.0f, GetContentRegionAvail().x - min_w);
			ImVec2 text_pos(window->DC.CursorPos.x, pos.y + window->DC.CurrLineTextBaseOffset);
			pressed = ImGui::Selectable("", menu_is_open, selectable_flags | ImGuiSelectableFlags_SpanAvailWidth, ImVec2(min_w, label_size.y));
			LogSetNextTextDecoration("", ">");
			RenderText(ImVec2(text_pos.x + offsets->OffsetLabel, text_pos.y), label);
			if (icon_w > 0.0f)
				RenderText(ImVec2(text_pos.x + offsets->OffsetIcon, text_pos.y), icon);
			RenderArrow(window->DrawList, ImVec2(text_pos.x + offsets->OffsetMark + extra_w + g.FontSize * 0.30f, text_pos.y), GetColorU32(ImGuiCol_Text), ImGuiDir_Right);
			popup_pos = ImVec2(pos.x, text_pos.y - style.WindowPadding.y);
		}
		if (!enabled)
			EndDisabled();

		const bool hovered = (g.HoveredId == id) && enabled && !g.NavHighlightItemUnderNav;
		hoveredPar = hovered;
		if (menuset_is_open)
			PopItemFlag();

		bool want_open = false;
		bool want_open_nav_init = false;
		bool want_close = false;
		if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
		{
			// Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
			// Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
			bool moving_toward_child_menu = false;
			ImGuiPopupData* child_popup = (g.BeginPopupStack.Size < g.OpenPopupStack.Size) ? &g.OpenPopupStack[g.BeginPopupStack.Size] : NULL; // Popup candidate (testing below)
			ImGuiWindow* child_menu_window = (child_popup && child_popup->Window && child_popup->Window->ParentWindow == window) ? child_popup->Window : NULL;
			if (g.HoveredWindow == window && child_menu_window != NULL) {
				const float ref_unit = g.FontSize; // FIXME-DPI
				const float child_dir = (window->Pos.x < child_menu_window->Pos.x) ? 1.0f : -1.0f;
				const ImRect next_window_rect = child_menu_window->Rect();
				ImVec2 ta = (g.IO.MousePos - g.IO.MouseDelta);
				ImVec2 tb = (child_dir > 0.0f) ? next_window_rect.GetTL() : next_window_rect.GetTR();
				ImVec2 tc = (child_dir > 0.0f) ? next_window_rect.GetBL() : next_window_rect.GetBR();
				const float pad_farmost_h = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, ref_unit * 0.5f, ref_unit * 2.5f); // Add a bit of extra slack.
				ta.x += child_dir * -0.5f;
				tb.x += child_dir * ref_unit;
				tc.x += child_dir * ref_unit;
				tb.y = ta.y + ImMax((tb.y - pad_farmost_h) - ta.y, -ref_unit * 8.0f); // Triangle has maximum height to limit the slope and the bias toward large sub-menus
				tc.y = ta.y + ImMin((tc.y + pad_farmost_h) - ta.y, +ref_unit * 8.0f);
				moving_toward_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
				//GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_toward_child_menu ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); // [DEBUG]
			}

			// The 'HovereWindow == window' check creates an inconsistency (e.g. moving away from menu slowly tends to hit same window, whereas moving away fast does not)
			// But we also need to not close the top-menu menu when moving over void. Perhaps we should extend the triangle check to a larger polygon.
			// (Remember to test this on BeginPopup("A")->BeginMenu("B") sequence which behaves slightly differently as B isn't a Child of A and hovering isn't shared.)
			if (menu_is_open && !hovered && g.HoveredWindow == window && !moving_toward_child_menu && !g.NavHighlightItemUnderNav && g.ActiveId == 0)
				want_close = true;

			// Open
			// (note: at this point 'hovered' actually includes the NavDisableMouseHover == false test)
			if (!menu_is_open && pressed) // Click/activate to open
				want_open = true;
			else if (!menu_is_open && hovered && !moving_toward_child_menu) // Hover to open
				want_open = true;
			else if (!menu_is_open && hovered && g.HoveredIdTimer >= 0.30f && g.MouseStationaryTimer >= 0.30f) // Hover to open (timer fallback)
				want_open = true;
			if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
			{
				want_open = want_open_nav_init = true;
				NavMoveRequestCancel();
				SetNavCursorVisibleAfterMove();
			}
		} else {
			// Menu bar
			if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
			{
				want_close = true;
				want_open = menu_is_open = false;
			} else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
			{
				want_open = true;
			} else if (g.NavId == id && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
			{
				want_open = true;
				NavMoveRequestCancel();
			}
		}

		if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
			want_close = true;
		if (want_close && IsPopupOpen(id, ImGuiPopupFlags_None))
			ClosePopupToLevel(g.BeginPopupStack.Size, true);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));
		PopID();

		if (want_open && !menu_is_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size) {
			// Don't reopen/recycle same menu level in the same frame if it is a different menu ID, first close the other menu and yield for a frame.
			OpenPopup(label);
		} else if (want_open) {
			menu_is_open = true;
			OpenPopup(label, ImGuiPopupFlags_NoReopen); // | (want_open_nav_init ? ImGuiPopupFlags_NoReopenAlwaysNavInit : 0));
		}

		if (menu_is_open) {
			ImGuiLastItemData last_item_in_parent = g.LastItemData;
			SetNextWindowPos(popup_pos, ImGuiCond_Always);                  // Note: misleading: the value will serve as reference for FindBestWindowPosForPopup(), not actual pos.
			PushStyleVar(ImGuiStyleVar_ChildRounding, style.PopupRounding); // First level will use _PopupRounding, subsequent will use _ChildRounding
			menu_is_open = BeginPopupMenuEx(id, label, window_flags);       // menu_is_open may be 'false' when the popup is completely clipped (e.g. zero size display)
			PopStyleVar();
			if (menu_is_open) {
				// Implement what ImGuiPopupFlags_NoReopenAlwaysNavInit would do:
				// Perform an init request in the case the popup was already open (via a previous mouse hover)
				if (want_open && want_open_nav_init && !g.NavInitRequest) {
					FocusWindow(g.CurrentWindow, ImGuiFocusRequestFlags_UnlessBelowModal);
					NavInitWindow(g.CurrentWindow, false);
				}

				// Restore LastItemData so IsItemXXXX functions can work after BeginMenu()/EndMenu()
				// (This fixes using IsItemClicked() and IsItemHovered(), but IsItemHovered() also relies on its support for ImGuiItemFlags_NoWindowHoverableCheck)
				g.LastItemData = last_item_in_parent;
				if (g.HoveredWindow == window)
					g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HoveredWindow;
			}
		} else {
			g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		}

		return menu_is_open;
	}

	bool BeginMenu(const char* label, bool enabled, bool& hoveredPar) {
		return BeginMenuEx(label, NULL, enabled, hoveredPar);
	}

	void BeginMenuChild(const char* child_str_id, const char* menu_label, std::function<void()> draw_func) {
		ImGui::BeginChild(child_str_id, ImVec2(0, ImGui::GetTextLineHeight()));
		bool hoveredChild = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows);

		if (ImGui::BeginMenu(menu_label)) {
			draw_func();

			bool hovoredMenu = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows);

			if (!hoveredChild && !hovoredMenu && !ImGui::IsAnyMouseDown()) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndMenu();
		}
		ImGui::EndChild();
	}

	void BeginMenu(const char* menu_label, std::function<void()> draw_func) {
		bool hovered;
		if (ImGuiEx::BeginMenu(menu_label, true, hovered)) {
			draw_func();

			if (!(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_ChildWindows) || hovered) && !ImGui::IsAnyMouseDown()) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndMenu();
		}
	}

	bool BeginPopupContextWindow(const char* str_id, ImGuiPopupFlags popup_flags, ImGuiHoveredFlags hovered_flags) {
		ImGuiWindow* window = GImGui->CurrentWindow;
		if (!str_id)
			str_id = "window_context";
		ImGuiID id = window->GetID(str_id);
		int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
		hovered_flags |= ImGuiHoveredFlags_AllowWhenBlockedByPopup;
		if (ImGui::IsMouseReleased(mouse_button) && ImGui::IsWindowHovered(hovered_flags))
			if (!(popup_flags & ImGuiPopupFlags_NoOpenOverItems) || !ImGui::IsAnyItemHovered())
				ImGui::OpenPopupEx(id, popup_flags);
		return ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
	}

	void MenuItemTableColumnVisibility(ImGuiTable* table, int columnIdx) {
		ImGuiTableColumn& column = table->Columns[columnIdx];
		const char* columnName = ImGui::TableGetColumnName(table, columnIdx);
		// Make sure we can't hide the last active column
		bool menu_item_active = (column.Flags & ImGuiTableColumnFlags_NoHide) ? false : true;
		if (column.IsEnabled && table->ColumnsEnabledCount <= 1)
			menu_item_active = false;
		if (ImGui::MenuItem(columnName, NULL, column.IsEnabled, menu_item_active))
			column.IsUserEnabledNextFrame = !column.IsEnabled;
	}

	ImRect TableGetCurrentRowRect() {
		ImGuiTable* table = GImGui->CurrentTable;
		assert(table != nullptr);
		ImRect row_rect(table->WorkRect.Min.x, table->RowPosY1, table->WorkRect.Max.x, table->RowPosY2);
		row_rect.ClipWith(table->BgClipRect);
		return row_rect;
	}

	bool TableIsMouseHoveringCurrentRow() {
		ImRect row_rect = TableGetCurrentRowRect();

		return ImGui::IsMouseHoveringRect(row_rect.Min, row_rect.Max, false);
	}

	bool WindowReposition(ImGuiWindow* window, Position position, const ImVec2& cornerVector, CornerPosition cornerPosition, ImGuiID fromWindowID, CornerPosition anchorPanelCornerPosition, CornerPosition selfPanelCornerPosition) {
		if (window == nullptr) {
			window = ImGui::GetCurrentWindowRead();
		}

		const ImVec2& windowSize = window->Size;
		const ImVec2& displaySize = ImGui::GetIO().DisplaySize;

		const ImVec2 startPos = window->Pos;

		switch (position) {
			case Position::ScreenRelative: {
				ImVec2 setPosition;
				const ImVec2& userPosition = cornerVector;
				switch (cornerPosition) {
					case CornerPosition::TopLeft: {
						setPosition = userPosition;
						break;
					}
					case CornerPosition::TopRight: {
						setPosition.x = displaySize.x - windowSize.x - userPosition.x;
						setPosition.y = userPosition.y;
						break;
					}
					case CornerPosition::BottomLeft: {
						setPosition.x = userPosition.x;
						setPosition.y = displaySize.y - windowSize.y - userPosition.y;
						break;
					}
					case CornerPosition::BottomRight: {
						setPosition = displaySize - windowSize - userPosition;
						break;
					}
				}

				ImGui::SetWindowPos(window, setPosition);
				break;
			}
			case Position::WindowRelative: {
				ImVec2 setPosition;
				const ImVec2& userPosition = cornerVector;
				ImGuiWindow* sourceWindow = ImGui::FindWindowByID(fromWindowID);
				if (!sourceWindow) {
					break;
				}
				const ImVec2& sourceWindowPosition = sourceWindow->Pos;
				const ImVec2& sourceWindowSize = sourceWindow->Size;
				switch (anchorPanelCornerPosition) {
					case CornerPosition::TopLeft: {
						setPosition = sourceWindowPosition;
						break;
					}
					case CornerPosition::TopRight: {
						setPosition.x = sourceWindowPosition.x + sourceWindowSize.x;
						setPosition.y = sourceWindowPosition.y;
						break;
					}
					case CornerPosition::BottomLeft: {
						setPosition.x = sourceWindowPosition.x;
						setPosition.y = sourceWindowPosition.y + sourceWindowSize.y;
						break;
					}
					case CornerPosition::BottomRight: {
						setPosition.x = sourceWindowPosition.x + sourceWindowSize.x;
						setPosition.y = sourceWindowPosition.y + sourceWindowSize.y;
						break;
					}
				}
				switch (selfPanelCornerPosition) {
					case CornerPosition::TopLeft: {
						setPosition = setPosition + userPosition;
						break;
					}
					case CornerPosition::TopRight: {
						setPosition.x = setPosition.x + userPosition.x - windowSize.x;
						setPosition.y = setPosition.y + userPosition.y;
						break;
					}
					case CornerPosition::BottomLeft: {
						setPosition.x = setPosition.x + userPosition.x;
						setPosition.y = setPosition.y + userPosition.y - windowSize.y;
						break;
					}
					case CornerPosition::BottomRight: {
						setPosition = setPosition + userPosition - windowSize;
						break;
					}
				}

				// clip to screen border
				setPosition = ImMax(setPosition, ImVec2(0.f, 0.f));

				ImGui::SetWindowPos(window, setPosition);
				break;
			}
			// nothing to do for Manual
			case Position::Manual: break;
		}

		const ImVec2& endPos = window->Pos;
		return startPos.x != endPos.x || startPos.y != endPos.y;
	}

	// Store ImGuiTreeNodeStackData for just submitted node.
	// Currently only supports 32 level deep and we are fine with (1 << Depth) overflowing into a zero, easy to increase.
	static void TreeNodeStoreStackData(ImGuiTreeNodeFlags flags, float x1)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		g.TreeNodeStack.resize(g.TreeNodeStack.Size + 1);
		ImGuiTreeNodeStackData* tree_node_data = &g.TreeNodeStack.Data[g.TreeNodeStack.Size - 1];
		tree_node_data->ID = g.LastItemData.ID;
		tree_node_data->TreeFlags = flags;
		tree_node_data->ItemFlags = g.LastItemData.ItemFlags;
		tree_node_data->NavRect = g.LastItemData.NavRect;

		// Initially I tried to latch value for GetColorU32(ImGuiCol_TreeLines) but it's not a good trade-off for very large trees.
		const bool draw_lines = (flags & (ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_DrawLinesToNodes)) != 0;
		tree_node_data->DrawLinesX1 = draw_lines ? (x1 + g.FontSize * 0.5f + g.Style.FramePadding.x) : +FLT_MAX;
		tree_node_data->DrawLinesTableColumn = (draw_lines && g.CurrentTable) ? (ImGuiTableColumnIdx) g.CurrentTable->CurrentColumn : -1;
		tree_node_data->DrawLinesToNodesY2 = -FLT_MAX;
		window->DC.TreeHasStackDataDepthMask |= (1 << window->DC.TreeDepth);
		if (flags & ImGuiTreeNodeFlags_DrawLinesToNodes)
			window->DC.TreeRecordsClippedNodesY2Mask |= (1 << window->DC.TreeDepth);
	}

	bool TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, void* icon) {
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// When not framed, we vertically increase height up to typical framed widget height
		const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
		const bool use_frame_padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding));
		const ImVec2 padding = use_frame_padding ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

		if (!label_end)
			label_end = FindRenderedTextEnd(label);
		const ImVec2 label_size = CalcTextSize(label, label_end, false);

		const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);                                                           // Collapsing arrow width + Spacing
		const float text_offset_y = use_frame_padding ? ImMax(style.FramePadding.y, window->DC.CurrLineTextBaseOffset) : window->DC.CurrLineTextBaseOffset; // Latch before ItemSize changes it
		const float text_width = g.FontSize + label_size.x + padding.x * 2;                                                                                 // Include collapsing arrow

		const float frame_height = label_size.y + padding.y * 2;
		const bool span_all_columns = (flags & ImGuiTreeNodeFlags_SpanAllColumns) != 0 && (g.CurrentTable != NULL);
		const bool span_all_columns_label = (flags & ImGuiTreeNodeFlags_LabelSpanAllColumns) != 0 && (g.CurrentTable != NULL);
		ImRect frame_bb;
		frame_bb.Min.x = span_all_columns ? window->ParentWorkRect.Min.x : (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x
																													  : window->DC.CursorPos.x;
		frame_bb.Min.y = window->DC.CursorPos.y + (text_offset_y - padding.y);
		frame_bb.Max.x = span_all_columns ? window->ParentWorkRect.Max.x : (flags & ImGuiTreeNodeFlags_SpanLabelWidth) ? window->DC.CursorPos.x + text_width + padding.x
																													   : window->WorkRect.Max.x;
		frame_bb.Max.y = window->DC.CursorPos.y + (text_offset_y - padding.y) + frame_height;
		if (display_frame) {
			const float outer_extend = IM_TRUNC(window->WindowPadding.x * 0.5f); // Framed header expand a little outside of current limits
			frame_bb.Min.x -= outer_extend;
			frame_bb.Max.x += outer_extend;
		}

		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		ItemSize(ImVec2(text_width, frame_height), padding.y);

		// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
		ImRect interact_bb = frame_bb;
		if ((flags & (ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_SpanLabelWidth | ImGuiTreeNodeFlags_SpanAllColumns)) == 0)
			interact_bb.Max.x = frame_bb.Min.x + text_width + (label_size.x > 0.0f ? style.ItemSpacing.x * 2.0f : 0.0f);

		// Compute open and multi-select states before ItemAdd() as it clear NextItem data.
		ImGuiID storage_id = (g.NextItemData.HasFlags & ImGuiNextItemDataFlags_HasStorageID) ? g.NextItemData.StorageId : id;
		bool is_open = TreeNodeUpdateNextOpen(storage_id, flags);

		bool is_visible;
		if (span_all_columns || span_all_columns_label) {
			// Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
			const float backup_clip_rect_min_x = window->ClipRect.Min.x;
			const float backup_clip_rect_max_x = window->ClipRect.Max.x;
			window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
			window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
			is_visible = ItemAdd(interact_bb, id);
			window->ClipRect.Min.x = backup_clip_rect_min_x;
			window->ClipRect.Max.x = backup_clip_rect_max_x;
		} else {
			is_visible = ItemAdd(interact_bb, id);
		}
		g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
		g.LastItemData.DisplayRect = frame_bb;

		// If a NavLeft request is happening and ImGuiTreeNodeFlags_NavLeftJumpsToParent enabled:
		// Store data for the current depth to allow returning to this node from any child item.
		// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
		// It will become tempting to enable ImGuiTreeNodeFlags_NavLeftJumpsToParent by default or move it to ImGuiStyle.
		bool store_tree_node_stack_data = false;
		if ((flags & ImGuiTreeNodeFlags_DrawLinesMask_) == 0)
			flags |= g.Style.TreeLinesFlags;
		const bool draw_tree_lines = (flags & (ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_DrawLinesToNodes)) && (frame_bb.Min.y < window->ClipRect.Max.y) && (g.Style.TreeLinesSize > 0.0f);
		if (!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen)) {
			store_tree_node_stack_data = draw_tree_lines;
			if ((flags & ImGuiTreeNodeFlags_NavLeftJumpsToParent) && !g.NavIdIsAlive)
				if (g.NavMoveDir == ImGuiDir_Left && g.NavWindow == window && NavMoveRequestButNoResultYet())
					store_tree_node_stack_data = true;
		}

		const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
		if (!is_visible) {
			if ((flags & ImGuiTreeNodeFlags_DrawLinesToNodes) && (window->DC.TreeRecordsClippedNodesY2Mask & (1 << (window->DC.TreeDepth - 1)))) {
				ImGuiTreeNodeStackData* parent_data = &g.TreeNodeStack.Data[g.TreeNodeStack.Size - 1];
				parent_data->DrawLinesToNodesY2 = ImMax(parent_data->DrawLinesToNodesY2, window->DC.CursorPos.y); // Don't need to aim to mid Y position as we are clipped anyway.
				if (frame_bb.Min.y >= window->ClipRect.Max.y)
					window->DC.TreeRecordsClippedNodesY2Mask &= ~(1 << (window->DC.TreeDepth - 1)); // Done
			}
			if (is_open && store_tree_node_stack_data)
				TreeNodeStoreStackData(flags, text_pos.x - text_offset_x); // Call before TreePushOverrideID()
			if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
				TreePushOverrideID(id);
			IMGUI_TEST_ENGINE_ITEM_INFO(g.LastItemData.ID, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
			return is_open;
		}

		if (span_all_columns || span_all_columns_label) {
			TablePushBackgroundChannel();
			g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
			g.LastItemData.ClipRect = window->ClipRect;
		}

		ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
		if ((flags & ImGuiTreeNodeFlags_AllowOverlap) || (g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap))
			button_flags |= ImGuiButtonFlags_AllowOverlap;
		if (!is_leaf)
			button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

		// We allow clicking on the arrow section with keyboard modifiers held, in order to easily
		// allow browsing a tree while preserving selection with code implementing multi-selection patterns.
		// When clicking on the rest of the tree node we always disallow keyboard modifiers.
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);

		const bool is_multi_select = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
		if (is_multi_select) // We absolutely need to distinguish open vs select so _OpenOnArrow comes by default
			flags |= (flags & ImGuiTreeNodeFlags_OpenOnMask_) == 0 ? ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick : ImGuiTreeNodeFlags_OpenOnArrow;

		// Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
		// Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
		// - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
		// - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
		// - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
		// It is rather standard that arrow click react on Down rather than Up.
		// We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
		if (is_mouse_x_over_arrow)
			button_flags |= ImGuiButtonFlags_PressedOnClick;
		else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
		else
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease;
		if (flags & ImGuiTreeNodeFlags_NoNavFocus)
			button_flags |= ImGuiButtonFlags_NoNavFocus;

		bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
		const bool was_selected = selected;

		// Multi-selection support (header)
		if (is_multi_select) {
			// Handle multi-select + alter button flags for it
			MultiSelectItemHeader(id, &selected, &button_flags);
			if (is_mouse_x_over_arrow)
				button_flags = (button_flags | ImGuiButtonFlags_PressedOnClick) & ~ImGuiButtonFlags_PressedOnClickRelease;
		} else {
			if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
				button_flags |= ImGuiButtonFlags_NoKeyModsAllowed;
		}

		bool hovered, held;
		bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
		bool toggled = false;
		if (!is_leaf) {
			if (pressed && g.DragDropHoldJustPressedId != id) {
				if ((flags & ImGuiTreeNodeFlags_OpenOnMask_) == 0 || (g.NavActivateId == id && !is_multi_select))
					toggled = true; // Single click
				if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
					toggled |= is_mouse_x_over_arrow && !g.NavHighlightItemUnderNav; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
				if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseClickedCount[0] == 2)
					toggled = true; // Double click
			} else if (pressed && g.DragDropHoldJustPressedId == id) {
				IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
				if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
					toggled = true;
				else
					pressed = false; // Cancel press so it doesn't trigger selection.
			}

			if (g.NavId == id && g.NavMoveDir == ImGuiDir_Left && is_open) {
				toggled = true;
				NavClearPreferredPosForAxis(ImGuiAxis_X);
				NavMoveRequestCancel();
			}
			if (g.NavId == id && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
			{
				toggled = true;
				NavClearPreferredPosForAxis(ImGuiAxis_X);
				NavMoveRequestCancel();
			}

			if (toggled) {
				is_open = !is_open;
				window->DC.StateStorage->SetInt(storage_id, is_open);
				g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
			}
		}

		// Multi-selection support (footer)
		if (is_multi_select) {
			bool pressed_copy = pressed && !toggled;
			MultiSelectItemFooter(id, &selected, &pressed_copy);
			if (pressed)
				SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, interact_bb);
		}

		if (selected != was_selected)
			g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		{
			const ImU32 text_col = GetColorU32(ImGuiCol_Text);
			ImGuiNavRenderCursorFlags nav_render_cursor_flags = ImGuiNavRenderCursorFlags_Compact;
			if (is_multi_select)
				nav_render_cursor_flags |= ImGuiNavRenderCursorFlags_AlwaysDraw; // Always show the nav rectangle
			if (display_frame) {
				// Framed type
				const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																									 : ImGuiCol_Header);
				RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
				RenderNavCursor(frame_bb, id, nav_render_cursor_flags);
				if (span_all_columns && !span_all_columns_label)
					TablePopBackgroundChannel();
				if (flags & ImGuiTreeNodeFlags_Bullet)
					RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
				else if (!is_leaf)
					RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ((flags & ImGuiTreeNodeFlags_UpsideDownArrow) ? ImGuiDir_Up : ImGuiDir_Down) : ImGuiDir_Right, 1.0f);
				else // Leaf without bullet, left-adjusted text
					text_pos.x -= text_offset_x - padding.x;

				if (icon) {
					float size = GetFontSize();
					Image(icon, ImVec2(size, size));
					SameLine();
				}

				if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
					frame_bb.Max.x -= g.FontSize + style.FramePadding.x;
				if (g.LogEnabled)
					LogSetNextTextDecoration("###", "###");
			} else {
				// Unframed typed for tree nodes
				if (hovered || selected) {
					const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																										 : ImGuiCol_Header);
					RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
				}
				RenderNavCursor(frame_bb, id, nav_render_cursor_flags);
				if (span_all_columns && !span_all_columns_label)
					TablePopBackgroundChannel();
				if (flags & ImGuiTreeNodeFlags_Bullet)
					RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
				else if (!is_leaf)
					RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ((flags & ImGuiTreeNodeFlags_UpsideDownArrow) ? ImGuiDir_Up : ImGuiDir_Down) : ImGuiDir_Right, 0.70f);
				if (g.LogEnabled)
					LogSetNextTextDecoration(">", NULL);				

				if (icon) {
					float size = GetFontSize();
					Image(icon, ImVec2(size, size));
					SameLine();
				}
			}

			if (draw_tree_lines)
				TreeNodeDrawLineToChildNode(ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.5f));

			// Label
			if (display_frame)
				RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
			else
				RenderText(text_pos, label, label_end, false);

			if (span_all_columns_label)
				TablePopBackgroundChannel();
		}

		if (is_open && store_tree_node_stack_data)
			TreeNodeStoreStackData(flags, text_pos.x - text_offset_x); // Call before TreePushOverrideID()
		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			TreePushOverrideID(id); // Could use TreePush(label) but this avoid computing twice

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
		return is_open;
	}

	bool TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags, void* icon) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return TreeNodeBehavior(window->GetID(label), flags, label, NULL, icon);
	}

	// Helper
	inline ImGuiTableFlags TableFixFlags(ImGuiTableFlags flags, ImGuiWindow* outer_window) {
		// Adjust flags: set default sizing policy
		if ((flags & ImGuiTableFlags_SizingMask_) == 0)
			flags |= ((flags & ImGuiTableFlags_ScrollX) || (outer_window->Flags & ImGuiWindowFlags_AlwaysAutoResize)) ? ImGuiTableFlags_SizingFixedFit : ImGuiTableFlags_SizingStretchSame;

		// Adjust flags: enable NoKeepColumnsVisible when using ImGuiTableFlags_SizingFixedSame
		if ((flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedSame)
			flags |= ImGuiTableFlags_NoKeepColumnsVisible;

		// Adjust flags: enforce borders when resizable
		if (flags & ImGuiTableFlags_Resizable)
			flags |= ImGuiTableFlags_BordersInnerV;

		// Adjust flags: disable NoHostExtendX/NoHostExtendY if we have any scrolling going on
		if (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
			flags &= ~(ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_NoHostExtendY);

		// Adjust flags: NoBordersInBodyUntilResize takes priority over NoBordersInBody
		if (flags & ImGuiTableFlags_NoBordersInBodyUntilResize)
			flags &= ~ImGuiTableFlags_NoBordersInBody;

		// Adjust flags: disable saved settings if there's nothing to save
		if ((flags & (ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Sortable)) == 0)
			flags |= ImGuiTableFlags_NoSavedSettings;

			// Inherit _NoSavedSettings from top-level window (child windows always have _NoSavedSettings set)
#ifdef IMGUI_HAS_DOCK
		ImGuiWindow* window_for_settings = outer_window->RootWindowDockStop;
#else
		ImGuiWindow* window_for_settings = outer_window->RootWindow;
#endif
		if (window_for_settings->Flags & ImGuiWindowFlags_NoSavedSettings)
			flags |= ImGuiTableFlags_NoSavedSettings;

		return flags;
	}

	bool BeginTableEx(const char* name, ImGuiID id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width, ImGuiWindowFlags child_window_flags) {
		ImGuiContext& g = *GImGui;
		ImGuiWindow* outer_window = GetCurrentWindow();
		if (outer_window->SkipItems) // Consistent with other tables + beneficial side effect that assert on miscalling EndTable() will be more visible.
			return false;

		// Sanity checks
		IM_ASSERT(columns_count > 0 && columns_count < IMGUI_TABLE_MAX_COLUMNS);
		if (flags & ImGuiTableFlags_ScrollX)
			IM_ASSERT(inner_width >= 0.0f);

		// If an outer size is specified ahead we will be able to early out when not visible. Exact clipping criteria may evolve.
		// FIXME: coarse clipping because access to table data causes two issues:
		// - instance numbers varying/unstable. may not be a direct problem for users, but could make outside access broken or confusing, e.g. TestEngine.
		// - can't implement support for ImGuiChildFlags_ResizeY as we need to somehow pull the height data from somewhere. this also needs stable instance numbers.
		// The side-effects of accessing table data on coarse clip would be:
		// - always reserving the pooled ImGuiTable data ahead for a fully clipped table (minor IMHO). Also the 'outer_window_is_measuring_size' criteria may already be defeating this in some situations.
		// - always performing the GetOrAddByKey() O(log N) query in g.Tables.Map[].
		const bool use_child_window = (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) != 0;
		const ImVec2 avail_size = GetContentRegionAvail();
		const ImVec2 actual_outer_size = ImTrunc(CalcItemSize(outer_size, ImMax(avail_size.x, IMGUI_WINDOW_HARD_MIN_SIZE), use_child_window ? ImMax(avail_size.y, IMGUI_WINDOW_HARD_MIN_SIZE) : 0.0f));
		const ImRect outer_rect(outer_window->DC.CursorPos, outer_window->DC.CursorPos + actual_outer_size);
		const bool outer_window_is_measuring_size = (outer_window->AutoFitFramesX > 0) || (outer_window->AutoFitFramesY > 0); // Doesn't apply to AlwaysAutoResize windows!
		if (use_child_window && IsClippedEx(outer_rect, 0) && !outer_window_is_measuring_size) {
			ItemSize(outer_rect);
			ItemAdd(outer_rect, id);
			g.NextWindowData.ClearFlags();
			return false;
		}

		// [DEBUG] Debug break requested by user
		if (g.DebugBreakInTable == id)
			IM_DEBUG_BREAK();

		// Acquire storage for the table
		ImGuiTable* table = g.Tables.GetOrAddByKey(id);

		// Acquire temporary buffers
		const int table_idx = g.Tables.GetIndex(table);
		if (++g.TablesTempDataStacked > g.TablesTempData.Size)
			g.TablesTempData.resize(g.TablesTempDataStacked, ImGuiTableTempData());
		ImGuiTableTempData* temp_data = table->TempData = &g.TablesTempData[g.TablesTempDataStacked - 1];
		temp_data->TableIndex = table_idx;
		table->DrawSplitter = &table->TempData->DrawSplitter;
		table->DrawSplitter->Clear();

		// Fix flags
		table->IsDefaultSizingPolicy = (flags & ImGuiTableFlags_SizingMask_) == 0;
		flags = TableFixFlags(flags, outer_window);

		// Initialize
		const int previous_frame_active = table->LastFrameActive;
		const int instance_no = (previous_frame_active != g.FrameCount) ? 0 : table->InstanceCurrent + 1;
		const ImGuiTableFlags previous_flags = table->Flags;
		table->ID = id;
		table->Flags = flags;
		table->LastFrameActive = g.FrameCount;
		table->OuterWindow = table->InnerWindow = outer_window;
		table->ColumnsCount = columns_count;
		table->IsLayoutLocked = false;
		table->InnerWidth = inner_width;
		table->NavLayer = (ImS8) outer_window->DC.NavLayerCurrent;
		temp_data->UserOuterSize = outer_size;

		// Instance data (for instance 0, TableID == TableInstanceID)
		ImGuiID instance_id;
		table->InstanceCurrent = (ImS16) instance_no;
		if (instance_no > 0) {
			IM_ASSERT(table->ColumnsCount == columns_count && "BeginTable(): Cannot change columns count mid-frame while preserving same ID");
			if (table->InstanceDataExtra.Size < instance_no)
				table->InstanceDataExtra.push_back(ImGuiTableInstanceData());
			instance_id = GetIDWithSeed(instance_no, GetIDWithSeed("##Instances", NULL, id)); // Push "##Instances" followed by (int)instance_no in ID stack.
		} else {
			instance_id = id;
		}
		ImGuiTableInstanceData* table_instance = TableGetInstanceData(table, table->InstanceCurrent);
		table_instance->TableInstanceID = instance_id;

		// When not using a child window, WorkRect.Max will grow as we append contents.
		if (use_child_window) {
			// Ensure no vertical scrollbar appears if we only want horizontal one, to make flag consistent
			// (we have no other way to disable vertical scrollbar of a window while keeping the horizontal one showing)
			ImVec2 override_content_size(FLT_MAX, FLT_MAX);
			if ((flags & ImGuiTableFlags_ScrollX) && !(flags & ImGuiTableFlags_ScrollY))
				override_content_size.y = FLT_MIN;

			// Ensure specified width (when not specified, Stretched columns will act as if the width == OuterWidth and
			// never lead to any scrolling). We don't handle inner_width < 0.0f, we could potentially use it to right-align
			// based on the right side of the child window work rect, which would require knowing ahead if we are going to
			// have decoration taking horizontal spaces (typically a vertical scrollbar).
			if ((flags & ImGuiTableFlags_ScrollX) && inner_width > 0.0f)
				override_content_size.x = inner_width;

			if (override_content_size.x != FLT_MAX || override_content_size.y != FLT_MAX)
				SetNextWindowContentSize(ImVec2(override_content_size.x != FLT_MAX ? override_content_size.x : 0.0f, override_content_size.y != FLT_MAX ? override_content_size.y : 0.0f));

			// Reset scroll if we are reactivating it
			if ((previous_flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) == 0)
				if ((g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasScroll) == 0)
					SetNextWindowScroll(ImVec2(0.0f, 0.0f));

			// Create scrolling region (without border and zero window padding)
			ImGuiChildFlags child_child_flags = (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasChildFlags) ? g.NextWindowData.ChildFlags : ImGuiChildFlags_None;
			child_window_flags |= (g.NextWindowData.HasFlags & ImGuiNextWindowDataFlags_HasWindowFlags) ? g.NextWindowData.WindowFlags : ImGuiWindowFlags_None;
			if (flags & ImGuiTableFlags_ScrollX)
				child_window_flags |= ImGuiWindowFlags_HorizontalScrollbar;
			BeginChildEx(name, instance_id, outer_rect.GetSize(), child_child_flags, child_window_flags);
			table->InnerWindow = g.CurrentWindow;
			table->WorkRect = table->InnerWindow->WorkRect;
			table->OuterRect = table->InnerWindow->Rect();
			table->InnerRect = table->InnerWindow->InnerRect;
			IM_ASSERT(table->InnerWindow->WindowPadding.x == 0.0f && table->InnerWindow->WindowPadding.y == 0.0f && table->InnerWindow->WindowBorderSize == 0.0f);

			// Allow submitting when host is measuring
			if (table->InnerWindow->SkipItems && outer_window_is_measuring_size)
				table->InnerWindow->SkipItems = false;

			// When using multiple instances, ensure they have the same amount of horizontal decorations (aka vertical scrollbar) so stretched columns can be aligned
			if (instance_no == 0) {
				table->HasScrollbarYPrev = table->HasScrollbarYCurr;
				table->HasScrollbarYCurr = false;
			}
			table->HasScrollbarYCurr |= table->InnerWindow->ScrollbarY;
		} else {
			// For non-scrolling tables, WorkRect == OuterRect == InnerRect.
			// But at this point we do NOT have a correct value for .Max.y (unless a height has been explicitly passed in). It will only be updated in EndTable().
			table->WorkRect = table->OuterRect = table->InnerRect = outer_rect;
			table->HasScrollbarYPrev = table->HasScrollbarYCurr = false;
			table->InnerWindow->DC.TreeDepth++; // This is designed to always linking ImGuiTreeNodeFlags_DrawLines linking across a table
		}

		// Push a standardized ID for both child-using and not-child-using tables
		PushOverrideID(id);
		if (instance_no > 0)
			PushOverrideID(instance_id); // FIXME: Somehow this is not resolved by stack-tool, even tho GetIDWithSeed() submitted the symbol.

		// Backup a copy of host window members we will modify
		ImGuiWindow* inner_window = table->InnerWindow;
		table->HostIndentX = inner_window->DC.Indent.x;
		table->HostClipRect = inner_window->ClipRect;
		table->HostSkipItems = inner_window->SkipItems;
		temp_data->WindowID = inner_window->ID;
		temp_data->HostBackupWorkRect = inner_window->WorkRect;
		temp_data->HostBackupParentWorkRect = inner_window->ParentWorkRect;
		temp_data->HostBackupColumnsOffset = outer_window->DC.ColumnsOffset;
		temp_data->HostBackupPrevLineSize = inner_window->DC.PrevLineSize;
		temp_data->HostBackupCurrLineSize = inner_window->DC.CurrLineSize;
		temp_data->HostBackupCursorMaxPos = inner_window->DC.CursorMaxPos;
		temp_data->HostBackupItemWidth = outer_window->DC.ItemWidth;
		temp_data->HostBackupItemWidthStackSize = outer_window->DC.ItemWidthStack.Size;
		inner_window->DC.PrevLineSize = inner_window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);

		// Make borders not overlap our contents by offsetting HostClipRect (#6765, #7428, #3752)
		// (we normally shouldn't alter HostClipRect as we rely on TableMergeDrawChannels() expanding non-clipped column toward the
		// limits of that rectangle, in order for ImDrawListSplitter::Merge() to merge the draw commands. However since the overlap
		// problem only affect scrolling tables in this case we can get away with doing it without extra cost).
		if (inner_window != outer_window) {
			// FIXME: Because inner_window's Scrollbar doesn't know about border size, since it's not encoded in window->WindowBorderSize,
			// it already overlaps it and doesn't need an extra offset. Ideally we should be able to pass custom border size with
			// different x/y values to BeginChild().
			if (flags & ImGuiTableFlags_BordersOuterV) {
				table->HostClipRect.Min.x = ImMin(table->HostClipRect.Min.x + TABLE_BORDER_SIZE, table->HostClipRect.Max.x);
				if (inner_window->DecoOuterSizeX2 == 0.0f)
					table->HostClipRect.Max.x = ImMax(table->HostClipRect.Max.x - TABLE_BORDER_SIZE, table->HostClipRect.Min.x);
			}
			if (flags & ImGuiTableFlags_BordersOuterH) {
				table->HostClipRect.Min.y = ImMin(table->HostClipRect.Min.y + TABLE_BORDER_SIZE, table->HostClipRect.Max.y);
				if (inner_window->DecoOuterSizeY2 == 0.0f)
					table->HostClipRect.Max.y = ImMax(table->HostClipRect.Max.y - TABLE_BORDER_SIZE, table->HostClipRect.Min.y);
			}
		}

		// Padding and Spacing
		// - None               ........Content..... Pad .....Content........
		// - PadOuter           | Pad ..Content..... Pad .....Content.. Pad |
		// - PadInner           ........Content.. Pad | Pad ..Content........
		// - PadOuter+PadInner  | Pad ..Content.. Pad | Pad ..Content.. Pad |
		const bool pad_outer_x = (flags & ImGuiTableFlags_NoPadOuterX) ? false : (flags & ImGuiTableFlags_PadOuterX) ? true
																													 : (flags & ImGuiTableFlags_BordersOuterV) != 0;
		const bool pad_inner_x = (flags & ImGuiTableFlags_NoPadInnerX) ? false : true;
		const float inner_spacing_for_border = (flags & ImGuiTableFlags_BordersInnerV) ? TABLE_BORDER_SIZE : 0.0f;
		const float inner_spacing_explicit = (pad_inner_x && (flags & ImGuiTableFlags_BordersInnerV) == 0) ? g.Style.CellPadding.x : 0.0f;
		const float inner_padding_explicit = (pad_inner_x && (flags & ImGuiTableFlags_BordersInnerV) != 0) ? g.Style.CellPadding.x : 0.0f;
		table->CellSpacingX1 = inner_spacing_explicit + inner_spacing_for_border;
		table->CellSpacingX2 = inner_spacing_explicit;
		table->CellPaddingX = inner_padding_explicit;

		const float outer_padding_for_border = (flags & ImGuiTableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
		const float outer_padding_explicit = pad_outer_x ? g.Style.CellPadding.x : 0.0f;
		table->OuterPaddingX = (outer_padding_for_border + outer_padding_explicit) - table->CellPaddingX;

		table->CurrentColumn = -1;
		table->CurrentRow = -1;
		table->RowBgColorCounter = 0;
		table->LastRowFlags = ImGuiTableRowFlags_None;
		table->InnerClipRect = (inner_window == outer_window) ? table->WorkRect : inner_window->ClipRect;
		table->InnerClipRect.ClipWith(table->WorkRect); // We need this to honor inner_width
		table->InnerClipRect.ClipWithFull(table->HostClipRect);
		table->InnerClipRect.Max.y = (flags & ImGuiTableFlags_NoHostExtendY) ? ImMin(table->InnerClipRect.Max.y, inner_window->WorkRect.Max.y) : table->HostClipRect.Max.y;

		table->RowPosY1 = table->RowPosY2 = table->WorkRect.Min.y; // This is needed somehow
		table->RowTextBaseline = 0.0f;                             // This will be cleared again by TableBeginRow()
		table->RowCellPaddingY = 0.0f;
		table->FreezeRowsRequest = table->FreezeRowsCount = 0; // This will be setup by TableSetupScrollFreeze(), if any
		table->FreezeColumnsRequest = table->FreezeColumnsCount = 0;
		table->IsUnfrozenRows = true;
		table->DeclColumnsCount = table->AngledHeadersCount = 0;
		if (previous_frame_active + 1 < g.FrameCount)
			table->IsActiveIdInTable = false;
		table->AngledHeadersHeight = 0.0f;
		temp_data->AngledHeadersExtraWidth = 0.0f;

		// Using opaque colors facilitate overlapping lines of the grid, otherwise we'd need to improve TableDrawBorders()
		table->BorderColorStrong = GetColorU32(ImGuiCol_TableBorderStrong);
		table->BorderColorLight = GetColorU32(ImGuiCol_TableBorderLight);

		// Make table current
		g.CurrentTable = table;
		inner_window->DC.NavIsScrollPushableX = false; // Shortcut for NavUpdateCurrentWindowIsScrollPushableX();
		outer_window->DC.CurrentTableIdx = table_idx;
		if (inner_window != outer_window) // So EndChild() within the inner window can restore the table properly.
			inner_window->DC.CurrentTableIdx = table_idx;

		if ((previous_flags & ImGuiTableFlags_Reorderable) && (flags & ImGuiTableFlags_Reorderable) == 0)
			table->IsResetDisplayOrderRequest = true;

		// Mark as used to avoid GC
		if (table_idx >= g.TablesLastTimeActive.Size)
			g.TablesLastTimeActive.resize(table_idx + 1, -1.0f);
		g.TablesLastTimeActive[table_idx] = (float) g.Time;
		temp_data->LastTimeActive = (float) g.Time;
		table->MemoryCompacted = false;

		// Setup memory buffer (clear data if columns count changed)
		ImGuiTableColumn* old_columns_to_preserve = NULL;
		void* old_columns_raw_data = NULL;
		const int old_columns_count = table->Columns.size();
		if (old_columns_count != 0 && old_columns_count != columns_count) {
			// Attempt to preserve width and other settings on column count/specs change (#4046)
			old_columns_to_preserve = table->Columns.Data;
			old_columns_raw_data = table->RawData; // Free at end of function
			table->RawData = NULL;
		}
		if (table->RawData == NULL) {
			TableBeginInitMemory(table, columns_count);
			table->IsInitializing = table->IsSettingsRequestLoad = true;
		}
		if (table->IsResetAllRequest)
			TableResetSettings(table);
		if (table->IsInitializing) {
			// Initialize
			table->SettingsOffset = -1;
			table->IsSortSpecsDirty = true;
			table->IsSettingsDirty = true; // Records itself into .ini file even when in default state (#7934)
			table->InstanceInteracted = -1;
			table->ContextPopupColumn = -1;
			table->ReorderColumn = table->ReorderColumnDstOrder = table->ResizedColumn = table->LastResizedColumn = -1;
			table->AutoFitSingleColumn = -1;
			table->HoveredColumnBody = table->HoveredColumnBorder = -1;
			for (int n = 0; n < columns_count; n++) {
				ImGuiTableColumn* column = &table->Columns[n];
				if (old_columns_to_preserve && n < old_columns_count) {
					*column = old_columns_to_preserve[n];
				} else {
					float width_auto = column->WidthAuto;
					*column = ImGuiTableColumn();
					column->WidthAuto = width_auto;
					column->IsPreserveWidthAuto = true; // Preserve WidthAuto when reinitializing a live table: not technically necessary but remove a visible flicker
					column->IsEnabled = column->IsUserEnabled = column->IsUserEnabledNextFrame = true;
					column->DisplayOrder = (ImGuiTableColumnIdx) n;
				}
				table->DisplayOrderToIndex[n] = column->DisplayOrder;
			}
		}
		if (old_columns_raw_data)
			IM_FREE(old_columns_raw_data);

		// Load settings
		if (table->IsSettingsRequestLoad)
			TableLoadSettings(table);

		// Handle DPI/font resize
		// This is designed to facilitate DPI changes with the assumption that e.g. style.CellPadding has been scaled as well.
		// It will also react to changing fonts with mixed results. It doesn't need to be perfect but merely provide a decent transition.
		// FIXME-DPI: Provide consistent standards for reference size. Perhaps using g.CurrentDpiScale would be more self explanatory.
		// This is will lead us to non-rounded WidthRequest in columns, which should work but is a poorly tested path.
		const float new_ref_scale_unit = g.FontSize; // g.Font->GetCharAdvance('A') ?
		if (table->RefScale != 0.0f && table->RefScale != new_ref_scale_unit) {
			const float scale_factor = new_ref_scale_unit / table->RefScale;
			//IMGUI_DEBUG_PRINT("[table] %08X RefScaleUnit %.3f -> %.3f, scaling width by %.3f\n", table->ID, table->RefScaleUnit, new_ref_scale_unit, scale_factor);
			for (int n = 0; n < columns_count; n++)
				table->Columns[n].WidthRequest = table->Columns[n].WidthRequest * scale_factor;
		}
		table->RefScale = new_ref_scale_unit;

		// Disable output until user calls TableNextRow() or TableNextColumn() leading to the TableUpdateLayout() call..
		// This is not strictly necessary but will reduce cases were "out of table" output will be misleading to the user.
		// Because we cannot safely assert in EndTable() when no rows have been created, this seems like our best option.
		inner_window->SkipItems = true;

		// Clear names
		// At this point the ->NameOffset field of each column will be invalid until TableUpdateLayout() or the first call to TableSetupColumn()
		if (table->ColumnsNames.Buf.Size > 0)
			table->ColumnsNames.Buf.resize(0);

		// Apply queued resizing/reordering/hiding requests
		TableBeginApplyRequests(table);

		return true;
	}

	bool BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width, ImGuiWindowFlags child_window_flags) {
		ImGuiID id = ImGui::GetID(str_id);
		// This could be ImGui::BeginTable() with g.NextWindowData.WindowFlags set to child_window_flags and g.NextWindowData.HasFlags set to ImGuiNextWindowDataFlags_HasWindowFlags.
		return BeginTableEx(str_id, id, columns_count, flags, outer_size, inner_width, child_window_flags);
	}

#ifdef _WIN32
	[[deprecated]] // use KeyInput.h instead
	void
	KeyInput(const char* label, const char* id, char* buffer, size_t bufSize, WPARAM& keyContainer, const char* notSetText) {
		ImGui::TextUnformatted(label);
		ImGui::SameLine();
		ImGui::PushItemWidth(30);
		if (ImGui::InputText(id, buffer, bufSize, ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank)) {
			size_t textLen = strlen(buffer);
			if (textLen == 0) {
				keyContainer = 0;
			} else if (textLen == 1) {
				// cut off the second byte, only the first one contains the vkeycode (https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-vkkeyscana)
				SHORT vkKeyScanA = VkKeyScanA(buffer[0]);
				keyContainer = (vkKeyScanA >> (8 * 0)) & 0xff;
			} else if (textLen == 2) {
				try {
					const int keyId = std::stoi(buffer);
					keyContainer = keyId;
				} catch ([[maybe_unused]] const std::invalid_argument& e) {
					keyContainer = 0;
				} catch ([[maybe_unused]] const std::out_of_range& e) {
					keyContainer = 0;
				}
			}
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();

		if (keyContainer == 0) {
			ImGui::TextUnformatted(notSetText);
		} else {
			// convert virtual key to vsc key
			UINT vscKey = MapVirtualKeyA(keyContainer, MAPVK_VK_TO_VSC);
			// get the name representation of the key
			char shortCutRealName[32]{};
			GetKeyNameTextA((vscKey << 16), shortCutRealName, 32);
			ImGui::TextUnformatted(shortCutRealName);
		}
	}
#endif
} // namespace ImGuiEx

#pragma warning(pop)
