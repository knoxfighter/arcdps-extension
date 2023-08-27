#pragma once

#include "../MainTable.h"

#include "../../arcdps_structs.h"

namespace ArcdpsExtension {
	using std::string_literals::operator""s;

	static const std::vector<MainTableColumn> tableColumns = {
			{0, [] { return "Column1"s; }, [] { return nullptr; }, "0"  },
			{1, [] { return "Column2"s; }, [] { return nullptr; }, "0"  },
			{2, [] { return "Column3"s; }, [] { return nullptr; }, "1"  },
			{3, [] { return "Column4"s; }, [] { return nullptr; }, "1.1"},
	};

	/**
 * Hardcoded rows, this is just here for this 
 */
	struct Row {
		std::string Column1;
		std::string Column2;
		std::string Column3;
		std::string Column4;
		std::string HoveredText;
	};

	static inline std::vector<Row> Rows = {
			{"1Row1.1", "4Row1.2", "3Row1.3", "3Row1.4", "Row1 Hovered Text"},
			{"2Row2.1", "3Row2.2", "5Row2.3", "1Row2.4", "Row2 Hovered Text"},
			{"3Row3.1", "5Row3.2", "2Row3.3", "5Row3.4", "Row3 Hovered Text"},
			{"4Row4.1", "2Row4.2", "1Row4.3", "4Row4.4", "Row4 Hovered Text"},
			{"5Row5.1", "1Row5.2", "4Row5.3", "2Row5.4", "Row5 Hovered Text"},
	};

	class DemoTable : public MainTable<> {
	public:
		explicit DemoTable(MainWindow* pMainWindow) : MainTable<>(tableColumns, pMainWindow, MainTableFlags_SubWindow) {}

	protected:
		Alignment& getAlignment() override;
		Alignment& getHeaderAlignment() override;
		std::string getTableId() override;
		int& getMaxDisplayed() override;
		const char* getCategoryName(const std::string& pCat) override;
		bool& getShowAlternatingBackground() override;
		TableSettings& getTableSettings() override;
		bool& getHighlightHoveredRows() override;
		bool& getShowHeaderAsText() override;

		void DrawRows(TableColumnIdx pFirstColumnIndex) override;
		void Sort(const ImGuiTableColumnSortSpecs* mColumnSortSpecs) override;

	private:
		Alignment mAlignment = Alignment::Left;
		Alignment mHeaderAlignment = Alignment::Left;
		int mMaxDisplayed = 0;
		bool mAlternatingBackground = false;
		TableSettings mSettings;
		bool mHighlightHoveredRows = true;
		bool mShowHeaderAsText = false;
	};
} // namespace ArcdpsExtension
