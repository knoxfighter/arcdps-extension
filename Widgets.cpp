#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "Widgets.h"

#include <cstdlib>
#include <stdexcept>

#include "ImGui_Math.h"

#pragma warning(push)
// disable warning for implicit conversion (for now)
#pragma warning(disable : 4244)

static const float TABLE_BORDER_SIZE = 1.0f; // hardcoded in imgui 1.80 as well

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
		IM_ASSERT(table != NULL && "Need to call TableHeader() after BeginTable()!");
		IM_ASSERT(table->CurrentColumn != -1);
		const int column_n = table->CurrentColumn;
		ImGuiTableColumn* column = &table->Columns[column_n];

		// Label
		if (label == NULL)
			label = "";
		const char* label_end = ImGui::FindRenderedTextEnd(label);
		ImVec2 label_size = ImGui::CalcTextSize(label, label_end, true);
		ImVec2 label_pos = window->DC.CursorPos;

		// If we already got a row height, there's use that.
		// FIXME-TABLE: Padding problem if the correct outer-padding CellBgRect strays off our ClipRect?
		ImRect cell_r = ImGui::TableGetCellBgRect(table, column_n);
		float label_height = table->RowMinHeight - table->CellPaddingY * 2.0f;
		if (show_label) {
			label_height = ImMax(label_size.y, label_height);
		} else {
			label_height = ImMax(image_size, label_height);
		}

		// Calculate ideal size for sort order arrow
		float w_arrow = 0.0f;
		float w_sort_text = 0.0f;
		char sort_order_suf[4] = "";
		const float ARROW_SCALE = 0.65f;
		if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
			w_arrow = ImFloor(g.FontSize * ARROW_SCALE + g.Style.FramePadding.x);
			if (column->SortOrder > 0) {
				ImFormatString(sort_order_suf, IM_ARRAYSIZE(sort_order_suf), "%d", column->SortOrder + 1);
				w_sort_text = g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(sort_order_suf).x;
			}
		}

		// We feed our unclipped width to the column without writing on CursorMaxPos, so that column is still considering for merging.
		float max_pos_x = label_pos.x + w_sort_text + w_arrow;
		if (show_label) {
			max_pos_x += label_size.x;
		} else {
			max_pos_x += image_size;
		}
		column->ContentMaxXHeadersUsed = ImMax(column->ContentMaxXHeadersUsed, column->WorkMaxX);
		column->ContentMaxXHeadersIdeal = ImMax(column->ContentMaxXHeadersIdeal, max_pos_x);

		// Keep header highlighted when context menu is open.
		const bool selected = (table->IsContextPopupOpen && table->ContextPopupColumn == column_n && table->InstanceInteracted == table->InstanceCurrent);
		ImGuiID id = window->GetID(label);
		ImRect bb(cell_r.Min.x, cell_r.Min.y, cell_r.Max.x, ImMax(cell_r.Max.y, cell_r.Min.y + label_height + g.Style.CellPadding.y * 2.0f));
		ImGui::ItemSize(ImVec2(0.0f, label_height)); // Don't declare unclipped width, it'll be fed ContentMaxPosHeadersIdeal
		if (!ImGui::ItemAdd(bb, id))
			return;

		//GetForegroundDrawList()->AddRect(cell_r.Min, cell_r.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]
		//GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(255, 0, 0, 255)); // [DEBUG]

		// Using AllowItemOverlap mode because we cover the whole cell, and we want user to be able to submit subsequent items.
		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_AllowItemOverlap);
		if (g.ActiveId != id)
			ImGui::SetItemAllowOverlap();
		if (held || hovered || selected) {
			const ImU32 col = ImGui::GetColorU32(held ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																						: ImGuiCol_Header);
			//RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, col, table->CurrentColumn);
			ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
		} else {
			// Submit single cell bg color in the case we didn't submit a full header row
			if ((table->RowFlags & ImGuiTableRowFlags_Headers) == 0)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg), table->CurrentColumn);
		}
		if (held)
			table->HeldHeaderColumn = (ImGuiTableColumnIdx) column_n;
		window->DC.CursorPos.y -= g.Style.ItemSpacing.y * 0.5f;

		// Drag and drop to re-order columns.
		// FIXME-TABLE: Scroll request while reordering a column and it lands out of the scrolling zone.
		if (held && (table->Flags & ImGuiTableFlags_Reorderable) && ImGui::IsMouseDragging(0) && !g.DragDropActive) {
			// While moving a column it will jump on the other side of the mouse, so we also test for MouseDelta.x
			table->ReorderColumn = (ImGuiTableColumnIdx) column_n;
			table->InstanceInteracted = table->InstanceCurrent;

			// We don't reorder: through the frozen<>unfrozen line, or through a column that is marked with ImGuiTableColumnFlags_NoReorder.
			if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < cell_r.Min.x)
				if (ImGuiTableColumn* prev_column = (column->PrevEnabledColumn != -1) ? &table->Columns[column->PrevEnabledColumn] : NULL)
					if (!((column->Flags | prev_column->Flags) & ImGuiTableColumnFlags_NoReorder))
						if ((column->IndexWithinEnabledSet < table->FreezeColumnsRequest) == (prev_column->IndexWithinEnabledSet < table->FreezeColumnsRequest))
							table->ReorderColumnDir = -1;
			if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > cell_r.Max.x)
				if (ImGuiTableColumn* next_column = (column->NextEnabledColumn != -1) ? &table->Columns[column->NextEnabledColumn] : NULL)
					if (!((column->Flags | next_column->Flags) & ImGuiTableColumnFlags_NoReorder))
						if ((column->IndexWithinEnabledSet < table->FreezeColumnsRequest) == (next_column->IndexWithinEnabledSet < table->FreezeColumnsRequest))
							table->ReorderColumnDir = +1;
		}

		// Sort order arrow
		const float ellipsis_max = cell_r.Max.x - w_arrow - w_sort_text;
		if ((table->Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
			if (column->SortOrder != -1) {
				float x = ImMax(cell_r.Min.x, cell_r.Max.x - w_arrow - w_sort_text);
				float y = label_pos.y;
				if (column->SortOrder > 0) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_Text, 0.70f));
					ImGui::RenderText(ImVec2(x + g.Style.ItemInnerSpacing.x, y), sort_order_suf);
					ImGui::PopStyleColor();
					x += w_sort_text;
				}
				ImGui::RenderArrow(window->DrawList, ImVec2(x, y), ImGui::GetColorU32(ImGuiCol_Text), column->SortDirection == ImGuiSortDirection_Ascending ? ImGuiDir_Up : ImGuiDir_Down, ARROW_SCALE);
			}

			// Handle clicking on column header to adjust Sort Order
			if (pressed && table->ReorderColumn != column_n) {
				ImGuiSortDirection sort_direction = ImGui::TableGetColumnNextSortDirection(column);
				ImGui::TableSetColumnSortDirection(column_n, sort_direction, g.IO.KeyShift);
			}
		}

		// Render clipped label. Clipping here ensure that in the majority of situations, all our header cells will
		// be merged into a single draw call.
		//window->DrawList->AddCircleFilled(ImVec2(ellipsis_max, label_pos.y), 40, IM_COL32_WHITE);
		if (show_label) {
			// ImGui::RenderTextEllipsis(window->DrawList, label_pos, ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max,
			// 	ellipsis_max, label, label_end, &label_size);

			float newX = label_pos.x;

			switch (alignment) {
				case Alignment::Center:
					// newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (label_size.x / 2);
					newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (label_size.x / 2);
					// ImGui::SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
					break;
				case Alignment::Right:
					newX = ellipsis_max - label_size.x;
					// ImGui::SetCursorPosX(cursorPosX + textSpace - contentSize.x);
					break;
			}

			ImGui::RenderTextEllipsis(window->DrawList, ImVec2(newX, label_pos.y), ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max, ellipsis_max, label, label_end, &label_size);
		} else {
			float newX = label_pos.x;

			switch (alignment) {
				case Alignment::Center:
					// newX = label_pos.x + ((ellipsis_max - label_pos.x) / 2) - (image_size / 2);
					newX = label_pos.x + ((cell_r.Max.x - label_pos.x - table->CellPaddingX) / 2) - (image_size / 2);
					// ImGui::SetCursorPosX(cursorPosX + (textSpace / 2 - contentSize.x / 2));
					break;
				case Alignment::Right:
					newX = ellipsis_max - image_size;
					// ImGui::SetCursorPosX(cursorPosX + textSpace - contentSize.x);
					break;
			}

			ImRect ibb(ImVec2(newX, label_pos.y), ImVec2(newX, label_pos.y) + image_size);

			window->DrawList->AddImage(texture, ibb.Min, ibb.Max);
		}

		// const bool text_clipped = label_size.x > (ellipsis_max - label_pos.x);
		// if (text_clipped && hovered && g.HoveredIdNotActiveTimer > g.TooltipSlowDelay)
		// ImGui::SetTooltip("%.*s", (int)(label_end - label), label);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", label);
		}

		// We don't use BeginPopupContextItem() because we want the popup to stay up even after the column is hidden
		// if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered())
		// ImGui::TableOpenContextMenu(column_n);
	}

	// This code can be used to make the text over the progressBar aligned.
	// This also uses imgui internals, which are likely to change between versions.
	void AlignedProgressBar(float fraction, const ImVec2& size_arg, const char* overlay, Alignment alignment) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
		ImRect bb(pos, pos + size);
		ImGui::ItemSize(size, style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, 0))
			return;

		// Render
		fraction = ImSaturate(fraction);
		ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
		bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
		const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
		ImGui::RenderRectFilledRangeH(window->DrawList, bb, ImGui::GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

		// Default displaying the fraction as percentage string, but user can override it
		char overlay_buf[32];
		if (!overlay) {
			ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction * 100 + 0.01f);
			overlay = overlay_buf;
		}

		ImVec2 overlay_size = ImGui::CalcTextSize(overlay, NULL);
		if (overlay_size.x > 0.0f) {
			switch (alignment) {
				case Alignment::Left:
					ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(0.f, 0.f), &bb);
					break;
				case Alignment::Center:
					ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(0.5f, 0.5f), &bb);
					break;
				case Alignment::Right:
					ImGui::RenderTextClipped(bb.Min, bb.Max, overlay, NULL, &overlay_size, ImVec2(1.f, 0.f), &bb);
					break;
				default:
					ImGui::RenderTextClipped(
							ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max,
							overlay, NULL, &overlay_size, ImVec2(0.0f, 0.5f), &bb
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
	bool BeginMenu(const char* label, bool enabled, bool& hoveredPar) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		bool menu_is_open = ImGui::IsPopupOpen(id, ImGuiPopupFlags_None);

		// Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
		ImGuiWindowFlags flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
		if (window->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_ChildMenu))
			flags |= ImGuiWindowFlags_ChildWindow;

		// If a menu with same the ID was already submitted, we will append to it, matching the behavior of Begin().
		// We are relying on a O(N) search - so O(N log N) over the frame - which seems like the most efficient for the expected small amount of BeginMenu() calls per frame.
		// If somehow this is ever becoming a problem we can switch to use e.g. ImGuiStorage mapping key to last frame used.
		if (g.MenusIdSubmittedThisFrame.contains(id)) {
			if (menu_is_open)
				menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
			else
				g.NextWindowData.ClearFlags(); // we behave like Begin() and need to consume those values
			return menu_is_open;
		}

		// Tag menu as used. Next time BeginMenu() with same ID is called it will append to existing menu
		g.MenusIdSubmittedThisFrame.push_back(id);

		ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		bool pressed;
		bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
		ImGuiWindow* backed_nav_window = g.NavWindow;
		if (menuset_is_open)
			g.NavWindow = window; // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

		// The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
		// However the final position is going to be different! It is chosen by FindBestWindowPosForPopup().
		// e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
		ImVec2 popup_pos, pos = window->DC.CursorPos;
		if (window->DC.LayoutType == ImGuiLayoutType_Horizontal) {
			// Menu inside an horizontal menu bar
			// Selectable extend their highlight by half ItemSpacing in each direction.
			// For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
			popup_pos = ImVec2(pos.x - 1.0f - IM_FLOOR(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
			window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * 0.5f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x * 2.0f, style.ItemSpacing.y));
			float w = label_size.x;
			pressed = ImGui::Selectable(label, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
			ImGui::PopStyleVar();
			window->DC.CursorPos.x += IM_FLOOR(style.ItemSpacing.x * (-1.0f + 0.5f));
			// -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
		} else {
			// Menu inside a menu
			// (In a typical menu window where all items are BeginMenu() or MenuItem() calls, extra_w will always be 0.0f.
			//  Only when they are other items sticking out we're going to add spacing, yet only register minimum width into the layout system.
			popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
			float min_w = window->DC.MenuColumns.DeclColumns(label_size.x, 0.0f, IM_FLOOR(g.FontSize * 1.20f)); // Feedback to next frame
			float extra_w = ImMax(0.0f, ImGui::GetContentRegionAvail().x - min_w);
			pressed = ImGui::Selectable(label, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_SelectOnClick | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_SpanAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(min_w, 0.0f));
			ImU32 text_col = ImGui::GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
			ImGui::RenderArrow(window->DrawList, pos + ImVec2(window->DC.MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f), text_col, ImGuiDir_Right);
		}

		const bool hovered = enabled && ImGui::ItemHoverable(window->DC.LastItemRect, id);
		hoveredPar = hovered;
		if (menuset_is_open)
			g.NavWindow = backed_nav_window;

		bool want_open = false;
		bool want_close = false;
		if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
		{
			// Close menu when not hovering it anymore unless we are moving roughly in the direction of the menu
			// Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
			bool moving_toward_other_child_menu = false;

			ImGuiWindow* child_menu_window = (g.BeginPopupStack.Size < g.OpenPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].SourceWindow == window)
													 ? g.OpenPopupStack[g.BeginPopupStack.Size].Window
													 : NULL;
			if (g.HoveredWindow == window && child_menu_window != NULL && !(window->Flags & ImGuiWindowFlags_MenuBar)) {
				// FIXME-DPI: Values should be derived from a master "scale" factor.
				ImRect next_window_rect = child_menu_window->Rect();
				ImVec2 ta = g.IO.MousePos - g.IO.MouseDelta;
				ImVec2 tb = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
				ImVec2 tc = (window->Pos.x < child_menu_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
				float extra = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, 5.0f, 30.0f);    // add a bit of extra slack.
				ta.x += (window->Pos.x < child_menu_window->Pos.x) ? -0.5f : +0.5f; // to avoid numerical issues
				tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);
				// triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
				tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
				moving_toward_other_child_menu = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
				//GetForegroundDrawList()->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); // [DEBUG]
			}
			if (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_toward_other_child_menu)
				want_close = true;

			if (!menu_is_open && hovered && pressed) // Click to open
				want_open = true;
			else if (!menu_is_open && hovered && !moving_toward_other_child_menu) // Hover to open
				want_open = true;

			if (g.NavActivateId == id) {
				want_close = menu_is_open;
				want_open = !menu_is_open;
			}
			if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
			{
				want_open = true;
				ImGui::NavMoveRequestCancel();
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
			} else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
			{
				want_open = true;
				ImGui::NavMoveRequestCancel();
			}
		}

		if (!enabled)
			// explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
			want_close = true;
		if (want_close && ImGui::IsPopupOpen(id, ImGuiPopupFlags_None))
			ImGui::ClosePopupToLevel(g.BeginPopupStack.Size, true);

		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));

		if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size) {
			// Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
			ImGui::OpenPopup(label);
			return false;
		}

		menu_is_open |= want_open;
		if (want_open)
			ImGui::OpenPopup(label);

		if (menu_is_open) {
			ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always);
			menu_is_open = ImGui::BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
		} else {
			g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
		}

		return menu_is_open;
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
			column.IsEnabledNextFrame = !column.IsEnabled;
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
		}

		const ImVec2& endPos = window->Pos;
		return startPos.x != endPos.x || startPos.y != endPos.y;
	}

	bool TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, void* icon) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
		const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

		if (!label_end)
			label_end = ImGui::FindRenderedTextEnd(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, label_end, false);

		// We vertically grow up to current line height up the typical widget height.
		const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
		ImRect frame_bb;
		frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
		frame_bb.Min.y = window->DC.CursorPos.y;
		frame_bb.Max.x = window->WorkRect.Max.x;
		frame_bb.Max.y = window->DC.CursorPos.y + frame_height;
		if (display_frame) {
			// Framed header expand a little outside the default padding, to the edge of InnerClipRect
			// (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
			frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
			frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
		}

		const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);          // Collapser arrow width + Spacing
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                   // Latch before ItemSize changes it
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f); // Include collapser
		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		ImGui::ItemSize(ImVec2(text_width, frame_height), padding.y);

		// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
		ImRect interact_bb = frame_bb;
		if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
			interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

		// Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
		// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
		// This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
		const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
		bool is_open = ImGui::TreeNodeBehaviorIsOpen(id, flags);
		if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

		bool item_add = ImGui::ItemAdd(interact_bb, id);
		window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
		window->DC.LastItemDisplayRect = frame_bb;

		if (!item_add) {
			if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
				ImGui::TreePushOverrideID(id);
			IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
			return is_open;
		}

		ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			button_flags |= ImGuiButtonFlags_AllowItemOverlap;
		if (!is_leaf)
			button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

		// We allow clicking on the arrow section with keyboard modifiers held, in order to easily
		// allow browsing a tree while preserving selection with code implementing multi-selection patterns.
		// When clicking on the rest of the tree node we always disallow keyboard modifiers.
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
		if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
			button_flags |= ImGuiButtonFlags_NoKeyModifiers;

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

		bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
		const bool was_selected = selected;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
		bool toggled = false;
		if (!is_leaf) {
			if (pressed && g.DragDropHoldJustPressedId != id) {
				if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
					toggled = true;
				if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
					toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
				if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
					toggled = true;
			} else if (pressed && g.DragDropHoldJustPressedId == id) {
				IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
				if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
					toggled = true;
			}

			if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open) {
				toggled = true;
				ImGui::NavMoveRequestCancel();
			}
			if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
			{
				toggled = true;
				ImGui::NavMoveRequestCancel();
			}

			if (toggled) {
				is_open = !is_open;
				window->DC.StateStorage->SetInt(id, is_open);
				window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
			}
		}
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			ImGui::SetItemAllowOverlap();

		// In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		const ImU32 text_col = ImGui::GetColorU32(ImGuiCol_Text);
		ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
		if (display_frame) {
			// Framed type
			const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																										: ImGuiCol_Header);
			ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
			ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			if (flags & ImGuiTreeNodeFlags_Bullet)
				ImGui::RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
			else if (!is_leaf)
				ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
			else // Leaf without bullet, left-adjusted text
				text_pos.x -= text_offset_x;
			if (icon) {
				float size = ImGui::GetFontSize();
				ImGui::Image(icon, ImVec2(size, size));
				ImGui::SameLine();
			}
			if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
				frame_bb.Max.x -= g.FontSize + style.FramePadding.x;
			if (g.LogEnabled) {
				// NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
				const char log_prefix[] = "\n##";
				const char log_suffix[] = "##";
				ImGui::LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
				ImGui::RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
				ImGui::LogRenderedText(&text_pos, log_suffix, log_suffix + 2);
			} else {
				ImGui::RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
			}
		} else {
			// Unframed typed for tree nodes
			if (hovered || selected) {
				const ImU32 bg_col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered
																											: ImGuiCol_Header);
				ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
				ImGui::RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			}
			if (flags & ImGuiTreeNodeFlags_Bullet)
				ImGui::RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
			else if (!is_leaf)
				ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
			if (g.LogEnabled)
				ImGui::LogRenderedText(&text_pos, ">");

			if (icon) {
				float size = ImGui::GetFontSize();
				ImGui::Image(icon, ImVec2(size, size));
				ImGui::SameLine();
			}

			ImGui::RenderText(text_pos, label, label_end, false);
		}

		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			ImGui::TreePushOverrideID(id);
		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
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
		ImGuiWindow* outer_window = ImGui::GetCurrentWindow();
		if (outer_window->SkipItems) // Consistent with other tables + beneficial side effect that assert on miscalling EndTable() will be more visible.
			return false;

		// Sanity checks
		IM_ASSERT(columns_count > 0 && columns_count <= IMGUI_TABLE_MAX_COLUMNS && "Only 1..64 columns allowed!");
		if (flags & ImGuiTableFlags_ScrollX)
			IM_ASSERT(inner_width >= 0.0f);

		// If an outer size is specified ahead we will be able to early out when not visible. Exact clipping rules may evolve.
		const bool use_child_window = (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) != 0;
		const ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImVec2 actual_outer_size = ImGui::CalcItemSize(outer_size, ImMax(avail_size.x, 1.0f), use_child_window ? ImMax(avail_size.y, 1.0f) : 0.0f);
		ImRect outer_rect(outer_window->DC.CursorPos, outer_window->DC.CursorPos + actual_outer_size);
		if (use_child_window && ImGui::IsClippedEx(outer_rect, 0, false)) {
			ImGui::ItemSize(outer_rect);
			return false;
		}

		// Acquire storage for the table
		ImGuiTable* table = g.Tables.GetOrAddByKey(id);
		const int instance_no = (table->LastFrameActive != g.FrameCount) ? 0 : table->InstanceCurrent + 1;
		const ImGuiID instance_id = id + instance_no;
		const ImGuiTableFlags table_last_flags = table->Flags;
		if (instance_no > 0)
			IM_ASSERT(table->ColumnsCount == columns_count && "BeginTable(): Cannot change columns count mid-frame while preserving same ID");

		// Fix flags
		table->IsDefaultSizingPolicy = (flags & ImGuiTableFlags_SizingMask_) == 0;
		flags = TableFixFlags(flags, outer_window);

		// Initialize
		table->ID = id;
		table->Flags = flags;
		table->InstanceCurrent = (ImS16) instance_no;
		table->LastFrameActive = g.FrameCount;
		table->OuterWindow = table->InnerWindow = outer_window;
		table->ColumnsCount = columns_count;
		table->IsLayoutLocked = false;
		table->InnerWidth = inner_width;
		table->UserOuterSize = outer_size;

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
				ImGui::SetNextWindowContentSize(ImVec2(override_content_size.x != FLT_MAX ? override_content_size.x : 0.0f, override_content_size.y != FLT_MAX ? override_content_size.y : 0.0f));

			// Reset scroll if we are reactivating it
			if ((table_last_flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) == 0)
				ImGui::SetNextWindowScroll(ImVec2(0.0f, 0.0f));

			// Create scrolling region (without border and zero window padding)
			ImGuiWindowFlags child_flags = child_window_flags;
			child_flags |= (flags & ImGuiTableFlags_ScrollX) ? ImGuiWindowFlags_HorizontalScrollbar : ImGuiWindowFlags_None;
			ImGui::BeginChildEx(name, instance_id, outer_rect.GetSize(), false, child_flags);
			table->InnerWindow = g.CurrentWindow;
			table->WorkRect = table->InnerWindow->WorkRect;
			table->OuterRect = table->InnerWindow->Rect();
			table->InnerRect = table->InnerWindow->InnerRect;
			IM_ASSERT(table->InnerWindow->WindowPadding.x == 0.0f && table->InnerWindow->WindowPadding.y == 0.0f && table->InnerWindow->WindowBorderSize == 0.0f);
		} else {
			// For non-scrolling tables, WorkRect == OuterRect == InnerRect.
			// But at this point we do NOT have a correct value for .Max.y (unless a height has been explicitly passed in). It will only be updated in EndTable().
			table->WorkRect = table->OuterRect = table->InnerRect = outer_rect;
		}

		// Push a standardized ID for both child-using and not-child-using tables
		ImGui::PushOverrideID(instance_id);

		// Backup a copy of host window members we will modify
		ImGuiWindow* inner_window = table->InnerWindow;
		table->HostIndentX = inner_window->DC.Indent.x;
		table->HostClipRect = inner_window->ClipRect;
		table->HostSkipItems = inner_window->SkipItems;
		table->HostBackupWorkRect = inner_window->WorkRect;
		table->HostBackupParentWorkRect = inner_window->ParentWorkRect;
		table->HostBackupColumnsOffset = outer_window->DC.ColumnsOffset;
		table->HostBackupPrevLineSize = inner_window->DC.PrevLineSize;
		table->HostBackupCurrLineSize = inner_window->DC.CurrLineSize;
		table->HostBackupCursorMaxPos = inner_window->DC.CursorMaxPos;
		table->HostBackupItemWidth = outer_window->DC.ItemWidth;
		table->HostBackupItemWidthStackSize = outer_window->DC.ItemWidthStack.Size;
		inner_window->DC.PrevLineSize = inner_window->DC.CurrLineSize = ImVec2(0.0f, 0.0f);

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
		table->CellPaddingY = g.Style.CellPadding.y;

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
		table->InnerClipRect.Max.y = (flags & ImGuiTableFlags_NoHostExtendY) ? ImMin(table->InnerClipRect.Max.y, inner_window->WorkRect.Max.y) : inner_window->ClipRect.Max.y;

		table->RowPosY1 = table->RowPosY2 = table->WorkRect.Min.y; // This is needed somehow
		table->RowTextBaseline = 0.0f;                             // This will be cleared again by TableBeginRow()
		table->FreezeRowsRequest = table->FreezeRowsCount = 0;     // This will be setup by TableSetupScrollFreeze(), if any
		table->FreezeColumnsRequest = table->FreezeColumnsCount = 0;
		table->IsUnfrozenRows = true;
		table->DeclColumnsCount = 0;

		// Using opaque colors facilitate overlapping elements of the grid
		table->BorderColorStrong = ImGui::GetColorU32(ImGuiCol_TableBorderStrong);
		table->BorderColorLight = ImGui::GetColorU32(ImGuiCol_TableBorderLight);

		// Make table current
		const int table_idx = g.Tables.GetIndex(table);
		g.CurrentTableStack.push_back(ImGuiPtrOrIndex(table_idx));
		g.CurrentTable = table;
		outer_window->DC.CurrentTableIdx = table_idx;
		if (inner_window != outer_window) // So EndChild() within the inner window can restore the table properly.
			inner_window->DC.CurrentTableIdx = table_idx;

		if ((table_last_flags & ImGuiTableFlags_Reorderable) && (flags & ImGuiTableFlags_Reorderable) == 0)
			table->IsResetDisplayOrderRequest = true;

		// Mark as used
		if (table_idx >= g.TablesLastTimeActive.Size)
			g.TablesLastTimeActive.resize(table_idx + 1, -1.0f);
		g.TablesLastTimeActive[table_idx] = (float) g.Time;
		table->MemoryCompacted = false;

		// Setup memory buffer (clear data if columns count changed)
		const int stored_size = table->Columns.size();
		if (stored_size != 0 && stored_size != columns_count) {
			IM_FREE(table->RawData);
			table->RawData = NULL;
		}
		if (table->RawData == NULL) {
			ImGui::TableBeginInitMemory(table, columns_count);
			table->IsInitializing = table->IsSettingsRequestLoad = true;
		}
		if (table->IsResetAllRequest)
			ImGui::TableResetSettings(table);
		if (table->IsInitializing) {
			// Initialize
			table->SettingsOffset = -1;
			table->IsSortSpecsDirty = true;
			table->InstanceInteracted = -1;
			table->ContextPopupColumn = -1;
			table->ReorderColumn = table->ResizedColumn = table->LastResizedColumn = -1;
			table->AutoFitSingleColumn = -1;
			table->HoveredColumnBody = table->HoveredColumnBorder = -1;
			for (int n = 0; n < columns_count; n++) {
				ImGuiTableColumn* column = &table->Columns[n];
				float width_auto = column->WidthAuto;
				*column = ImGuiTableColumn();
				column->WidthAuto = width_auto;
				column->IsPreserveWidthAuto = true; // Preserve WidthAuto when reinitializing a live table: not technically necessary but remove a visible flicker
				column->DisplayOrder = table->DisplayOrderToIndex[n] = (ImGuiTableColumnIdx) n;
				column->IsEnabled = column->IsEnabledNextFrame = true;
			}
		}

		// Load settings
		if (table->IsSettingsRequestLoad)
			ImGui::TableLoadSettings(table);

		// Handle DPI/font resize
		// This is designed to facilitate DPI changes with the assumption that e.g. style.CellPadding has been scaled as well.
		// It will also react to changing fonts with mixed results. It doesn't need to be perfect but merely provide a decent transition.
		// FIXME-DPI: Provide consistent standards for reference size. Perhaps using g.CurrentDpiScale would be more self explanatory.
		// This is will lead us to non-rounded WidthRequest in columns, which should work but is a poorly tested path.
		const float new_ref_scale_unit = g.FontSize; // g.Font->GetCharAdvance('A') ?
		if (table->RefScale != 0.0f && table->RefScale != new_ref_scale_unit) {
			const float scale_factor = new_ref_scale_unit / table->RefScale;
			//IMGUI_DEBUG_LOG("[table] %08X RefScaleUnit %.3f -> %.3f, scaling width by %.3f\n", table->ID, table->RefScaleUnit, new_ref_scale_unit, scale_factor);
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
		ImGui::TableBeginApplyRequests(table);

		return true;
	}

	bool BeginTable(const char* str_id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width, ImGuiWindowFlags child_window_flags) {
		ImGuiID id = ImGui::GetID(str_id);
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
