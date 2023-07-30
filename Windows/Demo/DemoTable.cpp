#include "DemoTable.h"

Alignment& DemoTable::getAlignment() {
	return mAlignment;
}

Alignment& DemoTable::getHeaderAlignment() {
	return mHeaderAlignment;
}

std::string DemoTable::getTableId() {
	return "DemoTableId";
}

int& DemoTable::getMaxDisplayed() {
	return mMaxDisplayed;
}

const char* DemoTable::getCategoryName(const std::string& pCat) {
	if (pCat == "1") return "Cat1";
	if (pCat == "1.1") return "Cat1.1";
	return "";
}

bool& DemoTable::getShowAlternatingBackground() {
	return mAlternatingBackground;
}

MainTable<64>::TableSettings& DemoTable::getTableSettings() {
	return mSettings;
}

bool& DemoTable::getHighlightHoveredRows() {
	return mHighlightHoveredRows;
}

bool& DemoTable::getShowHeaderAsText() {
	return mShowHeaderAsText;
}

void DemoTable::DrawRows(TableColumnIdx pFirstColumnIndex) {
	for (const Row& row : Rows) {
		NextRow();

		NextColumn();
		AlignedTextColumn(row.Column1.c_str());

		NextColumn();
		AlignedTextColumn(row.Column2.c_str());

		NextColumn();
		AlignedTextColumn(row.Column3.c_str());

		NextColumn();
		AlignedTextColumn(row.Column4.c_str());

		EndMaxHeightRow();

		if (IsCurrentRowHovered()) {
			ARC_LOG("IsCurrentRowHovered!");
			ImGui::SetTooltip("%s", row.HoveredText.c_str());
		}
	}
}

void DemoTable::Sort(const ImGuiTableColumnSortSpecs* mColumnSortSpecs) {
	const bool descend = mColumnSortSpecs->SortDirection == ImGuiSortDirection_Descending;
	std::ranges::sort(Rows, [descend, mColumnSortSpecs](const Row& row1, const Row& row2) -> bool {
		switch (mColumnSortSpecs->ColumnIndex) {
			case 0: return descend ? row1.Column1 < row2.Column1 : row1.Column1 > row2.Column1;
			case 1: return descend ? row1.Column2 < row2.Column2 : row1.Column2 > row2.Column2;
			case 2: return descend ? row1.Column3 < row2.Column3 : row1.Column3 > row2.Column3;
			case 3: return descend ? row1.Column4 < row2.Column4 : row1.Column4 > row2.Column4;
		}
		return false;
	});
}
