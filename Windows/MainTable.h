#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "MainWindow.h"

#include "../arcdps_structs.h"
#include "../ExtensionTranslations.h"
#include "../ImGui_Math.h"
#include "../Localization.h"
#include "../Widgets.h"

#include <algorithm>
#include <bitset>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

// Disable conversion warnings in this file
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

namespace ArcdpsExtension {
	static constexpr int TABLE_DRAW_CHANNEL_BG0 = 0;
	static constexpr int TABLE_DRAW_CHANNEL_BG2_FROZEN = 1;
	static constexpr int TABLE_DRAW_CHANNEL_NOCLIP = 2; // When using ImGuiTableFlags_NoClip (this becomes the last visible channel)

	static constexpr float TABLE_BORDER_SIZE = 1.0f;                      // hardcoded in imgui 1.80 as well
	static constexpr float TABLE_RESIZE_SEPARATOR_HALF_THICKNESS = 4.0f;  // Extend outside inner borders.
	static constexpr float TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER = 0.06f; // Delay/timer before making the hover feedback (color+cursor) visible because tables/columns tends to be more cramped.

	/**
	 * Column data type.
	 * This also defines the max amount of available columns.
	 * It always has to be signed, cause ImGui is using -1.
	 *
	 * Max of int16: 32767
	 */
	typedef ImS16 TableColumnIdx;

	/**
	 * Use this to define the columns.
	 * The constructor takes a vector of this.
	 * This is used to create the table, and the header.
	 *
	 * Currently not supported things:
	 * > ImGuiTableFlags_ContextMenuInBody -> This is not needed with this class.
	 */
	struct MainTableColumn {
		ImU32 UserId = 0;
		std::function<std::string()> Name;
		std::function<void*()> Texture;
		std::string Category; // The category for this Column in context menu. `0` is top-level. Only one sublevel is currently supported, e.g. `1.1`.
		bool DefaultVisibility = true;
		std::function<std::string()> Popup;

		MainTableColumn(ImU32 pUserId, const std::function<std::string()>& pName, const std::function<void*()>& pTexture, std::string pCategory, bool pDefaultVisibility = true)
			: UserId(pUserId),
			  Name(pName),
			  Texture(pTexture),
			  Category(std::move(pCategory)),
			  DefaultVisibility(pDefaultVisibility),
			  Popup(pName) {}

		template<typename E>
		requires std::is_enum_v<E> && std::convertible_to<std::underlying_type_t<E>, ImU32>
		MainTableColumn(E pUserId, const std::function<std::string()>& pName, const std::function<void*()>& pTexture, std::string pCategory, bool pDefaultVisibility = true)
			: MainTableColumn(std::to_underlying(pUserId), pName, pTexture, pCategory, pDefaultVisibility) {}

		MainTableColumn(ImU32 pUserId, const std::function<std::string()>& pName, const std::function<void*()>& pTexture, std::string pCategory, const std::function<std::string()>& pPopup, bool pDefaultVisibility = true)
			: UserId(pUserId),
			  Name(pName),
			  Texture(pTexture),
			  Category(std::move(pCategory)),
			  DefaultVisibility(pDefaultVisibility),
			  Popup(pPopup) {}

		template<typename E>
		requires std::is_enum_v<E> && std::convertible_to<std::underlying_type_t<E>, ImU32>
		MainTableColumn(E pUserId, const std::function<std::string()>& pName, const std::function<void*()>& pTexture, std::string pCategory, const std::function<std::string()>& pPopup, bool pDefaultVisibility = true)
			: MainTableColumn(std::to_underlying(pUserId), pName, pTexture, pCategory, pPopup, pDefaultVisibility) {}
	};

	enum MainTableFlags_ {
		MainTableFlags_None = 0,
		MainTableFlags_SubWindow = 1 << 0, // Needed if you want to want the table to have max-height. Window is put in a subwindow and will have scrollbars if needed.
	};
	typedef int MainTableFlags;

	// Used to define the Max MaxColumnCount.
	template<size_t T>
	concept SmallerThanMaxColumnAmount = requires {
		requires T < std::numeric_limits<TableColumnIdx>::max();
	};

	/**
	 * This class also contains the whole ImGui implementation of a table!
	 * It is a copy of the Table implementation of ImGui.
	 *
	 * The template is used to define the max size of the table.
	 * This is needed cause the ImGui table was changed to use `std::bitset`
	 *
	 * How to use:
	 * - Implement your own version of this class with all virtual void functions implemented.
	 * - Use your derived class in your window.
	 * - Make sure this class is constructed AFTER other components, so it hooks last.
	 * - `pColumns` in the constructor is saved as reference in this class, the calling object has to be valid the whole lifetime of this class!
	 * - Call `Draw()` in `DrawContent()`
	 * - Call `DrawColumnSetupMenu()` in `DrawContextMenu()` if you want to hide your columns.
	 * - Use `MigrateSettings` when you made changes to the table, that need the settings to be migrated. NEVER migrate within the Settings object itself, cause that would be too early.
	 *
	 * How do settings work:<br>
	 * Settings are still similar to how ImGui has designed them to be.
	 * The biggest difference is that columns are now saved based on the UserID instead of the index.
	 * The uniqueness of that ID has to be handled by the caller.
	 * When Migrating settings, delete what you removed, when adding new elements, only migrate the (still indexbased) DisplayOrder.
	 * When removing elements, only migrate the DisplayOrder as well.
	 * That is not all, there are some weird byproducts with the TableFlags and the SavedFlags.<br>
	 * !!! TEST THE SETTINGS MIGRATION REALLY WELL, IT IS SOMETIMES UNPREDICTABLE !!!
	 */
	template<size_t MaxColumnCount = 64>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	class MainTable {
	public:
		typedef std::bitset<MaxColumnCount> ColumnBitMask;

		MainTable(const std::vector<MainTableColumn>& pColumns, MainWindow* pMainWindow, MainTableFlags pFlags = 0);

		virtual ~MainTable() = default;

		// copy/move
		MainTable(const MainTable& pOther) = delete;
		MainTable(MainTable&& pOther) noexcept = delete;
		MainTable& operator=(const MainTable& pOther) = delete;
		MainTable& operator=(MainTable&& pOther) noexcept = delete;

		void Draw();
		void DrawColumnSetupMenu();
		static size_t GetMaxColumnCount() { return MaxColumnCount; }
		void RequestSort() { mSortNeeded = true; }

		void SetSpecificColumnSetup(const std::vector<size_t>& pNewColumns);
		void ResetSpecificColumnSetup();
		bool GetSpecificColumnsActive() const {
			return mSpecificColumnsActive;
		}

		struct TableColumnSettings {
			float WidthOrWeight = 0.0f;
			ImGuiID UserID = 0;
			// TableColumnIdx          Index = -1;
			TableColumnIdx DisplayOrder = -1;
			TableColumnIdx SortOrder = -1;
			ImU8 SortDirection : 2 = ImGuiSortDirection_None;
			ImU8 IsEnabled : 1 = 1; // "Visible" in ini file
			ImU8 IsStretch : 1 = 0;

			std::strong_ordering operator<=>(const TableColumnSettings& pOther) const {
				return UserID <=> pOther.UserID;
			}

			std::strong_ordering operator<=>(const ImGuiID& pOther) const {
				return UserID <=> pOther;
			}

			bool operator==(const TableColumnSettings& pOther) const {
				return UserID == pOther.UserID;
			}

			bool operator==(const ImGuiID& pOther) const {
				return UserID == pOther;
			}

			friend void to_json(nlohmann::json& nlohmann_json_j, const TableColumnSettings& nlohmann_json_t) {
				nlohmann_json_j["WidthOrWeight"] = nlohmann_json_t.WidthOrWeight;
				nlohmann_json_j["UserID"] = nlohmann_json_t.UserID;
				nlohmann_json_j["DisplayOrder"] = nlohmann_json_t.DisplayOrder;
				nlohmann_json_j["SortOrder"] = nlohmann_json_t.SortOrder;
				nlohmann_json_j["SortDirection"] = nlohmann_json_t.SortDirection;
				nlohmann_json_j["IsEnabled"] = nlohmann_json_t.IsEnabled;
				nlohmann_json_j["IsStretch"] = nlohmann_json_t.IsStretch;
			}

			friend void from_json(const nlohmann::json& nlohmann_json_j, TableColumnSettings& nlohmann_json_t) {
				nlohmann_json_j.at("WidthOrWeight").get_to(nlohmann_json_t.WidthOrWeight);
				nlohmann_json_j.at("UserID").get_to(nlohmann_json_t.UserID);
				nlohmann_json_j.at("DisplayOrder").get_to(nlohmann_json_t.DisplayOrder);
				nlohmann_json_j.at("SortOrder").get_to(nlohmann_json_t.SortOrder);
				nlohmann_json_t.SortDirection = nlohmann_json_j.at("SortDirection").get<ImU8>();
				nlohmann_json_t.IsEnabled = nlohmann_json_j.at("IsEnabled").get<ImU8>();
				nlohmann_json_t.IsStretch = nlohmann_json_j.at("IsStretch").get<ImU8>();
			}
		};
		struct TableSettings {
			ImU32 Version = 0;             // Is set to true if the ImGui Ini was migrated.
			ImGuiTableFlags SaveFlags = 0; // Indicate data we want to save using the Resizable/Reorderable/Sortable/Hideable flags (could be using its own flags..)
			float RefScale = 0;            // Reference scale to be able to rescale columns on font/dpi changes.
			std::vector<TableColumnSettings> Columns;

			NLOHMANN_DEFINE_TYPE_INTRUSIVE(TableSettings, Version, SaveFlags, RefScale, Columns)
		};

	protected:
		// Draw Rows here
		virtual void DrawRows(TableColumnIdx pFirstColumnIndex) = 0;

		/**
		 * This is called, when the table has to be resorted!
		 * - mColumnSortSpecs is guaranteed to be a valid pointer.
		 * - mSortNeeded is already set back to false.
		 */
		virtual void Sort(const ImGuiTableColumnSortSpecs* mColumnSortSpecs) = 0;

		virtual void DrawStyleSubMenu();
		virtual void DrawColumnSetupSubMenu();

		virtual Alignment& getAlignment() = 0;
		virtual Alignment& getHeaderAlignment() = 0;
		virtual bool getShowScrollbar() { return mMainWindow->GetShowScrollbar(); }
		virtual std::string getTableId() = 0;
		virtual bool getMaxHeightActive() { return getMaxDisplayed() != 0; }
		virtual int& getMaxDisplayed() = 0;
		virtual bool& getShowAlternatingBackground() = 0;
		virtual TableSettings& getTableSettings() = 0;
		virtual bool& getHighlightHoveredRows() = 0;
		virtual bool& getShowHeaderAsText() = 0;

		/**
		 * Set this to true if you want to use the feature "custom columns".
		 * Set a custom column setup with `SetSpecificColumnSetup`.
		 * If you want to go back to normal columns call `ResetSpecificColumnSetup`.
		 * While custom columns are active the settings saving is disabled.
		 */
		virtual bool getCustomColumnsFeatureActive() { return false; }
		virtual bool& getCustomColumnsActive() { return mCustomColumnsActiveTemp; }
		// Used to ignore the first N columns, so they are not changed.
		virtual int getCustomColumnsFirstColumn() { return 0; }

		/**
		 * Called when the migration of this TableSetting should be done.
		 *
		 * Also migrate the settings when you ADD new columns, else they have the absolute default values (which will show them)
		 */
		virtual void MigrateSettings() {}

		/**
		 * Get the name of each column category.
		 * Categories are defined in `MainTableColumn`.
		 */
		virtual const char* getCategoryName(const std::string& pCat) = 0;

	private:
		MainWindow* mMainWindow;
		const std::vector<MainTableColumn>& mColumns;
		int mCurrentRow = 0;
		std::atomic_bool mSortNeeded = false;
		MainTableFlags mFlags = 0;

		struct Category {
			std::vector<size_t> Indices;
			std::map<size_t, Category> Categories;
		};
		Category mCategories;

		void DrawColumnSetupMenuIteratorFunc(Category pCategory, std::string pCatName = "");

		std::atomic_bool mSpecificColumnsUpdate = false;
		bool mSpecificColumnsActive = false;
		std::vector<size_t> mSpecificColumnCache;

		bool mCustomColumnsActiveTemp = false; // only here to allow users to have the custom columns feature disabled.

		// Utilities to control ImGui, use these instead of ImGui directly!
	protected:
		/**
		 * Set the next Row, will call `ImGui::TableNextRow`
		 */
		void NextRow(ImGuiTableRowFlags row_flags = 0, float min_row_height = 0.0f); // append into the first cell of a new row.

		/**
		 * Set the next column, will call `ImGui::TableNextColumn`
		 */
		bool NextColumn();

		/**
		 * Call this at the end of each Row.
		 * This is used to calculate max-heights.
		 * Only call it on rows, you want to add to the max displayed amount. (e.g. killproof.me plugin doesn't add linked account rows)
		 */
		void EndMaxHeightRow();

		// Return NULL if no sort specs (most often when ImGuiTableFlags_Sortable is not set)
		// You can sort your data again when 'SpecsChanged == true'. It will be true with sorting specs have changed since
		// last call, or the first time.
		// Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable()!
		ImGuiTableSortSpecs* GetSortSpecs();

		// See "COLUMN SIZING POLICIES" comments at the top of this file
		// If (init_width_or_weight <= 0.0f) it is ignored
		void SetupColumn(const char* label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_id);
		void ColumnHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment, const char* popupText);

		/**
		 * Print text aligned to the current column.
		 * Use so text is aligned within the table.
		 * Alternatively use `ImGui::Text()` directly.
		 */
		void AlignedTextColumn(const char* text);
		void AlignedTextColumn(const std::string& text);
		template<typename... Args>
		void AlignedTextColumn(std::string_view format, Args&&... args);

		/**
		 * Aligned Spinner
		 */
		bool SpinnerAligned(const char* label, float radius, float thickness, const ImU32& color);

		void ApplySpecificColumnSetup();
		bool Begin(const char* str_id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width, ImGuiWindowFlags child_window_flags);
		void End();

		void MenuItemColumnVisibility(int TableColumnIdx);
		const char* GetColumnName(int column_n);
		void SetupScrollFreeze(int columns, int rows);

		// Return the cell rectangle based on currently known height.
		// - Important: we generally don't know our row height until the end of the row, so Max.y will be incorrect in many situations.
		//   The only case where this is correct is if we provided a min_row_height to TableNextRow() and don't go below it.
		// - Important: if ImGuiTableFlags_PadOuterX is set but ImGuiTableFlags_PadInnerX is not set, the outer-most left and right
		//   columns report a small offset so their CellBgRect can extend up to the outer border.
		ImRect GetCellBgRect(int column_n);

		/**
		 * Check if the current table column is hovered.
		 */
		bool IsCurrentColumnHovered();

		/**
		 * Check if the current table row is hovered.
		 */
		bool IsCurrentRowHovered();

		int GetColumnIndex();

		// We use the terminology "Enabled" to refer to a column that is not Hidden by user/api.
		// We use the terminology "Clipped" to refer to a column that is out of sight because of scrolling/clipping.
		// This is in contrast with some user-facing api such as IsItemVisible() / IsRectVisible() which use "Visible" to mean "not clipped".
		struct TableColumn {
			ImGuiTableColumnFlags Flags; // Flags after some patching (not directly same as provided by user). See ImGuiTableColumnFlags_
			float WidthGiven;            // Final/actual width visible == (MaxX - MinX), locked in TableUpdateLayout(). May be > WidthRequest to honor minimum width, may be < WidthRequest to honor shrinking columns down in tight space.
			float MinX;                  // Absolute positions
			float MaxX;
			float WidthRequest;             // Master width absolute value when !(Flags & _WidthStretch). When Stretch this is derived every frame from StretchWeight in TableUpdateLayout()
			float WidthAuto;                // Automatic width
			float StretchWeight;            // Master width weight when (Flags & _WidthStretch). Often around ~1.0f initially.
			float InitStretchWeightOrWidth; // Value passed to TableSetupColumn(). For Width it is a content width (_without padding_).
			ImRect ClipRect;                // Clipping rectangle for the column
			ImGuiID UserID;                 // Optional, value passed to TableSetupColumn()
			float WorkMinX;                 // Contents region min ~(MinX + CellPaddingX + CellSpacingX1) == cursor start position when entering column
			float WorkMaxX;                 // Contents region max ~(MaxX - CellPaddingX - CellSpacingX2)
			float ItemWidth;                // Current item width for the column, preserved across rows
			float ContentMaxXFrozen;        // Contents maximum position for frozen rows (apart from headers), from which we can infer content width.
			float ContentMaxXUnfrozen;
			float ContentMaxXHeadersUsed; // Contents maximum position for headers rows (regardless of freezing). TableHeader() automatically softclip itself + report ideal desired size, to avoid creating extraneous draw calls
			float ContentMaxXHeadersIdeal;
			ImS16 NameOffset;                            // Offset into parent ColumnsNames[]
			TableColumnIdx DisplayOrder;                 // Index within Table's IndexToDisplayOrder[] (column may be reordered by users)
			TableColumnIdx IndexWithinEnabledSet;        // Index within enabled/visible set (<= IndexToDisplayOrder)
			TableColumnIdx PrevEnabledColumn;            // Index of prev enabled/visible column within Columns[], -1 if first enabled/visible column
			TableColumnIdx NextEnabledColumn;            // Index of next enabled/visible column within Columns[], -1 if last enabled/visible column
			TableColumnIdx SortOrder;                    // Index of this column within sort specs, -1 if not sorting on this column, 0 for single-sort, may be >0 on multi-sort
			ImGuiTableDrawChannelIdx DrawChannelCurrent; // Index within DrawSplitter.Channels[]
			ImGuiTableDrawChannelIdx DrawChannelFrozen;
			ImGuiTableDrawChannelIdx DrawChannelUnfrozen;
			bool IsEnabled; // Is the column not marked Hidden by the user? (even if off view, e.g. clipped by scrolling).
			bool IsEnabledNextFrame;
			bool IsVisibleX; // Is actually in view (e.g. overlapping the host window clipping rectangle, not scrolled).
			bool IsVisibleY;
			bool IsRequestOutput; // Return value for TableSetColumnIndex() / TableNextColumn(): whether we request user to output contents or not.
			bool IsSkipItems;     // Do we want item submissions to this column to be completely ignored (no layout will happen).
			bool IsPreserveWidthAuto;
			ImS8 NavLayerCurrent;              // ImGuiNavLayer in 1 byte
			ImU8 AutoFitQueue;                 // Queue of 8 values for the next 8 frames to request auto-fit
			ImU8 CannotSkipItemsQueue;         // Queue of 8 values for the next 8 frames to disable Clipped/SkipItem
			ImU8 SortDirection : 2;            // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending
			ImU8 SortDirectionsAvailCount : 2; // Number of available sort directions (0 to 3)
			ImU8 SortDirectionsAvailMask : 4;  // Mask of available sort directions (1-bit each)
			ImU8 SortDirectionsAvailList;      // Ordered of available sort directions (2-bits each)

			TableColumn() {
				memset(this, 0, sizeof(*this));
				StretchWeight = WidthRequest = -1.0f;
				NameOffset = -1;
				DisplayOrder = IndexWithinEnabledSet = -1;
				PrevEnabledColumn = NextEnabledColumn = -1;
				SortOrder = -1;
				SortDirection = ImGuiSortDirection_None;
				DrawChannelCurrent = DrawChannelFrozen = DrawChannelUnfrozen = (ImU8) -1;
			}
		};

		// FIXME-TABLE: transient data could be stored in a per-stacked table structure: DrawSplitter, SortSpecs, incoming RowData
		struct Table {
			ImGuiID ID;
			ImGuiTableFlags Flags;
			void* RawData;                              // Single allocation to hold Columns[], DisplayOrderToIndex[] and RowCellData[]
			ImSpan<TableColumn> Columns;                // Point within RawData[]
			ImSpan<TableColumnIdx> DisplayOrderToIndex; // Point within RawData[]. Store display order of columns (when not reordered, the values are 0...Count-1)
			ImSpan<ImGuiTableCellData> RowCellData;     // Point within RawData[]. Store cells background requests for current row.
			ColumnBitMask EnabledMaskByDisplayOrder;    // Column DisplayOrder -> IsEnabled map
			ColumnBitMask EnabledMaskByIndex;           // Column Index -> IsEnabled map (== not hidden by user/api) in a format adequate for iterating column without touching cold data
			ColumnBitMask VisibleMaskByIndex;           // Column Index -> IsVisibleX|IsVisibleY map (== not hidden by user/api && not hidden by scrolling/cliprect)
			ColumnBitMask RequestOutputMaskByIndex;     // Column Index -> IsVisible || AutoFit (== expect user to submit items)
			ImGuiTableFlags SettingsLoadedFlags;        // Which data were loaded from the .ini file (e.g. when order is not altered we won't save order)
			int SettingsOffset;                         // Offset in g.SettingsTables
			int LastFrameActive;
			int ColumnsCount; // Number of columns declared in BeginTable()
			int CurrentRow;
			int CurrentHoveredRow; // Row that is currently hovered
			int CurrentColumn;
			ImS16 InstanceCurrent;    // Count of BeginTable() calls with same ID in the same frame (generally 0). This is a little bit similar to BeginCount for a window, but multiple table with same ID look are multiple tables, they are just synched.
			ImS16 InstanceInteracted; // Mark which instance (generally 0) of the same ID is being interacted with
			float RowPosY1;
			float RowPosY2;
			float RowMinHeight; // Height submitted to TableNextRow()
			float RowTextBaseline;
			float RowIndentOffsetX;
			ImGuiTableRowFlags RowFlags : 16; // Current row flags, see ImGuiTableRowFlags_
			ImGuiTableRowFlags LastRowFlags : 16;
			int RowBgColorCounter; // Counter for alternating background colors (can be fast-forwarded by e.g clipper), not same as CurrentRow because header rows typically don't increase this.
			ImU32 RowBgColor[2];   // Background color override for current row.
			ImU32 BorderColorStrong;
			ImU32 BorderColorLight;
			float BorderX1;
			float BorderX2;
			float HostIndentX;
			float MinColumnWidth;
			float OuterPaddingX;
			float CellPaddingX; // Padding from each borders
			float CellPaddingY;
			float CellSpacingX1; // Spacing between non-bordered cells
			float CellSpacingX2;
			float LastOuterHeight;     // Outer height from last frame
			float LastFirstRowHeight;  // Height of first row from last frame
			float InnerWidth;          // User value passed to BeginTable(), see comments at the top of BeginTable() for details.
			float ColumnsGivenWidth;   // Sum of current column width
			float ColumnsAutoFitWidth; // Sum of ideal column width in order nothing to be clipped, used for auto-fitting and content width submission in outer window
			float ResizedColumnNextWidth;
			float ResizeLockMinContentsX2; // Lock minimum contents width while resizing down in order to not create feedback loops. But we allow growing the table.
			float RefScale;                // Reference scale to be able to rescale columns on font/dpi changes.
			ImRect OuterRect;              // Note: for non-scrolling table, OuterRect.Max.y is often FLT_MAX until EndTable(), unless a height has been specified in BeginTable().
			ImRect InnerRect;              // InnerRect but without decoration. As with OuterRect, for non-scrolling tables, InnerRect.Max.y is
			ImRect WorkRect;
			ImRect InnerClipRect;
			ImRect BgClipRect;                // We use this to cpu-clip cell background color fill
			ImRect Bg0ClipRectForDrawCmd;     // Actual ImDrawCmd clip rect for BG0/1 channel. This tends to be == OuterWindow->ClipRect at BeginTable() because output in BG0/BG1 is cpu-clipped
			ImRect Bg2ClipRectForDrawCmd;     // Actual ImDrawCmd clip rect for BG2 channel. This tends to be a correct, tight-fit, because output to BG2 are done by widgets relying on regular ClipRect.
			ImRect HostClipRect;              // This is used to check if we can eventually merge our columns draw calls into the current draw call of the current window.
			ImRect HostBackupWorkRect;        // Backup of InnerWindow->WorkRect at the end of BeginTable()
			ImRect HostBackupParentWorkRect;  // Backup of InnerWindow->ParentWorkRect at the end of BeginTable()
			ImRect HostBackupInnerClipRect;   // Backup of InnerWindow->ClipRect during PushTableBackground()/PopTableBackground()
			ImVec2 HostBackupPrevLineSize;    // Backup of InnerWindow->DC.PrevLineSize at the end of BeginTable()
			ImVec2 HostBackupCurrLineSize;    // Backup of InnerWindow->DC.CurrLineSize at the end of BeginTable()
			ImVec2 HostBackupCursorMaxPos;    // Backup of InnerWindow->DC.CursorMaxPos at the end of BeginTable()
			ImVec2 UserOuterSize;             // outer_size.x passed to BeginTable()
			ImVec1 HostBackupColumnsOffset;   // Backup of OuterWindow->DC.ColumnsOffset at the end of BeginTable()
			float HostBackupItemWidth;        // Backup of OuterWindow->DC.ItemWidth at the end of BeginTable()
			int HostBackupItemWidthStackSize; // Backup of OuterWindow->DC.ItemWidthStack.Size at the end of BeginTable()
			ImGuiWindow* OuterWindow;         // Parent window for the table
			ImGuiWindow* InnerWindow;         // Window holding the table data (== OuterWindow or a child window)
			ImGuiTextBuffer ColumnsNames;     // Contiguous buffer holding columns names
			ImDrawListSplitter DrawSplitter;  // We carry our own ImDrawList splitter to allow recursion (FIXME: could be stored outside, worst case we need 1 splitter per recursing table)
			ImGuiTableColumnSortSpecs SortSpecsSingle;
			ImVector<ImGuiTableColumnSortSpecs> SortSpecsMulti; // FIXME-OPT: Using a small-vector pattern would work be good.
			ImGuiTableSortSpecs SortSpecs;                      // Public facing sorts specs, this is what we return in TableGetSortSpecs()
			TableColumnIdx SortSpecsCount;
			TableColumnIdx ColumnsEnabledCount;             // Number of enabled columns (<= ColumnsCount)
			TableColumnIdx ColumnsEnabledFixedCount;        // Number of enabled columns (<= ColumnsCount)
			TableColumnIdx DeclColumnsCount;                // Count calls to TableSetupColumn()
			TableColumnIdx HoveredColumnBody;               // Index of column whose visible region is being hovered. Important: == ColumnsCount when hovering empty region after the right-most column!
			TableColumnIdx HoveredColumnBorder;             // Index of column whose right-border is being hovered (for resizing).
			TableColumnIdx AutoFitSingleColumn;             // Index of single column requesting auto-fit.
			TableColumnIdx ResizedColumn;                   // Index of column being resized. Reset when InstanceCurrent==0.
			TableColumnIdx LastResizedColumn;               // Index of column being resized from previous frame.
			TableColumnIdx HeldHeaderColumn;                // Index of column header being held.
			TableColumnIdx ReorderColumn;                   // Index of column being reordered. (not cleared)
			TableColumnIdx ReorderColumnDir;                // -1 or +1
			TableColumnIdx LeftMostStretchedColumn;         // Index of left-most stretched column.
			TableColumnIdx RightMostStretchedColumn;        // Index of right-most stretched column.
			TableColumnIdx RightMostEnabledColumn;          // Index of right-most non-hidden column.
			TableColumnIdx ContextPopupColumn;              // Column right-clicked on, of -1 if opening context menu from a neutral/empty spot
			TableColumnIdx FreezeRowsRequest;               // Requested frozen rows count
			TableColumnIdx FreezeRowsCount;                 // Actual frozen row count (== FreezeRowsRequest, or == 0 when no scrolling offset)
			TableColumnIdx FreezeColumnsRequest;            // Requested frozen columns count
			TableColumnIdx FreezeColumnsCount;              // Actual frozen columns count (== FreezeColumnsRequest, or == 0 when no scrolling offset)
			TableColumnIdx RowCellDataCurrent;              // Index of current RowCellData[] entry in current row
			ImGuiTableDrawChannelIdx DummyDrawChannel;      // Redirect non-visible columns here.
			ImGuiTableDrawChannelIdx Bg2DrawChannelCurrent; // For Selectable() and other widgets drawing accross columns after the freezing line. Index within DrawSplitter.Channels[]
			ImGuiTableDrawChannelIdx Bg2DrawChannelUnfrozen;
			bool IsLayoutLocked; // Set by TableUpdateLayout() which is called when beginning the first row.
			bool IsInsideRow;    // Set when inside TableBeginRow()/TableEndRow().
			bool IsInitializing;
			bool IsSortSpecsDirty;
			bool IsUsingHeaders;     // Set when the first row had the ImGuiTableRowFlags_Headers flag.
			bool IsContextPopupOpen; // Set when default context menu is open (also see: ContextPopupColumn, InstanceInteracted).
			bool IsSettingsRequestLoad;
			bool IsSettingsDirty;       // Set when table settings have changed and needs to be reported into ImGuiTableSetttings data.
			bool IsDefaultDisplayOrder; // Set when display order is unchanged from default (DisplayOrder contains 0...Count-1)
			bool IsResetAllRequest;
			bool IsResetDisplayOrderRequest;
			bool IsUnfrozenRows;        // Set when we got past the frozen row.
			bool IsDefaultSizingPolicy; // Set if user didn't explicitely set a sizing policy in BeginTable()
			bool MemoryCompacted;
			bool HostSkipItems; // Backup of InnerWindow->SkipItem at the end of BeginTable(), because we will overwrite InnerWindow->SkipItem on a per-column basis

			IMGUI_API Table() {
				memset(this, 0, sizeof(*this));
				LastFrameActive = -1;
			}
			IMGUI_API ~Table() { IM_FREE(RawData); }
		};

		Table mTable;

	private:
		static ImGuiTableFlags TableFixFlags(ImGuiTableFlags flags, ImGuiWindow* outer_window);

		// For reference, the average total _allocation count_ for a table is:
		// + 0 (for ImGuiTable instance, we are pooling allocations in g.Tables)
		// + 1 (for table->RawData allocated below)
		// + 1 (for table->ColumnsNames, if names are used)
		// + 1 (for table->Splitter._Channels)
		// + 2 * active_channels_count (for ImDrawCmd and ImDrawIdx buffers inside channels)
		// Where active_channels_count is variable but often == columns_count or columns_count + 1, see TableSetupDrawChannels() for details.
		// Unused channels don't perform their +2 allocations.
		void BeginInitMemory(int columns_count);

		// Restore initial state of table (with or without saved settings)
		void ResetSettings();
		void LoadSettingsCustom();
		void SaveSettingsCustom();
		void MigrateIniSettings();

		// Apply queued resizing/reordering/hiding requests
		void BeginApplyRequests();

		// Get settings for a given table, NULL if none
		ImGuiTableSettings* GetBoundSettings();

		// Layout columns for the frame. This is in essence the followup to BeginTable().
		// Runs on the first call to TableNextRow(), to give a chance for TableSetupColumn() to be called first.
		// FIXME-TABLE: Our width (and therefore our WorkRect) will be minimal in the first frame for _WidthAuto columns.
		// Increase feedback side-effect with widgets relying on WorkRect.Max.x... Maybe provide a default distribution for _WidthAuto columns?
		void UpdateLayout();

		void SortSpecsBuild();

		// Adjust flags: default width mode + stretch columns are not allowed when auto extending
		void TableSetupColumnFlags(TableColumn* column, ImGuiTableColumnFlags flags_in);

		void EndRow();

		// FIXME-TABLE: This is a mess, need to redesign how we render borders (as some are also done in TableEndRow)
		void DrawBorders();

		// This function reorder draw channels based on matching clip rectangle, to facilitate merging them. Called by EndTable().
		// For simplicity we call it TableMergeDrawChannels() but in fact it only reorder channels + overwrite ClipRect,
		// actual merging is done by table->DrawSplitter.Merge() which is called right after TableMergeDrawChannels().
		//
		// Columns where the contents didn't stray off their local clip rectangle can be merged. To achieve
		// this we merge their clip rect and make them contiguous in the channel list, so they can be merged
		// by the call to DrawSplitter.Merge() following to the call to this function.
		// We reorder draw commands by arranging them into a maximum of 4 distinct groups:
		//
		//   1 group:               2 groups:              2 groups:              4 groups:
		//   [ 0. ] no freeze       [ 0. ] row freeze      [ 01 ] col freeze      [ 01 ] row+col freeze
		//   [ .. ]  or no scroll   [ 2. ]  and v-scroll   [ .. ]  and h-scroll   [ 23 ]  and v+h-scroll
		//
		// Each column itself can use 1 channel (row freeze disabled) or 2 channels (row freeze enabled).
		// When the contents of a column didn't stray off its limit, we move its channels into the corresponding group
		// based on its position (within frozen rows/columns groups or not).
		// At the end of the operation our 1-4 groups will each have a ImDrawCmd using the same ClipRect.
		// This function assume that each column are pointing to a distinct draw channel,
		// otherwise merge_group->ChannelsCount will not match set bit count of merge_group->ChannelsMask.
		//
		// Column channels will not be merged into one of the 1-4 groups in the following cases:
		// - The contents stray off its clipping rectangle (we only compare the MaxX value, not the MinX value).
		//   Direct ImDrawList calls won't be taken into account by default, if you use them make sure the ImGui:: bounds
		//   matches, by e.g. calling SetCursorScreenPos().
		// - The channel uses more than one draw command itself. We drop all our attempt at merging stuff here..
		//   we could do better but it's going to be rare and probably not worth the hassle.
		// Columns for which the draw channel(s) haven't been merged with other will use their own ImDrawCmd.
		//
		// This function is particularly tricky to understand.. take a breath.
		void MergeDrawChannels();

		// Note this is meant to be stored in column->WidthAuto, please generally use the WidthAuto field
		float GetColumnWidthAuto(TableColumn* column);

		// Adjust flags: default width mode + stretch columns are not allowed when auto extending
		void SetupColumnFlags(TableColumn* column, ImGuiTableColumnFlags flags_in);

		// Maximum column content width given current layout. Use column->MinX so this value on a per-column basis.
		float GetMaxColumnWidth(int column_n);

		// Allocate draw channels. Called by TableUpdateLayout()
		// - We allocate them following storage order instead of display order so reordering columns won't needlessly
		//   increase overall dormant memory cost.
		// - We isolate headers draw commands in their own channels instead of just altering clip rects.
		//   This is in order to facilitate merging of draw commands.
		// - After crossing FreezeRowsCount, all columns see their current draw channel changed to a second set of channels.
		// - We only use the dummy draw channel so we can push a null clipping rectangle into it without affecting other
		//   channels, while simplifying per-row/per-cell overhead. It will be empty and discarded when merged.
		// - We allocate 1 or 2 background draw channels. This is because we know TablePushBackgroundChannel() is only used for
		//   horizontal spanning. If we allowed vertical spanning we'd need one background draw channel per merge group (1-4).
		// Draw channel allocation (before merging):
		// - NoClip                       --> 2+D+1 channels: bg0/1 + bg2 + foreground (same clip rect == always 1 draw call)
		// - Clip                         --> 2+D+N channels
		// - FreezeRows                   --> 2+D+N*2 (unless scrolling value is zero)
		// - FreezeRows || FreezeColunns  --> 3+D+N*2 (unless scrolling value is zero)
		// Where D is 1 if any column is clipped or hidden (dummy channel) otherwise 0.
		void SetupDrawChannels();

		// Process hit-testing on resizing borders. Actual size change will be applied in EndTable()
		// - Set table->HoveredColumnBorder with a short delay/timer to reduce feedback noise
		// - Submit ahead of table contents and header, use ImGuiButtonFlags_AllowItemOverlap to prioritize widgets
		//   overlapping the same area.
		void UpdateBorders();

		void SortSpecsSanitize();

		// Fix sort direction if currently set on a value which is unavailable (e.g. activating NoSortAscending/NoSortDescending)
		void FixColumnSortDirection(TableColumn* column);

		static ImGuiSortDirection GetColumnAvailSortDirection(TableColumn* column, int n) {
			IM_ASSERT(n < column->SortDirectionsAvailCount);
			return (column->SortDirectionsAvailList >> (n << 1)) & 0x03;
		}

		void EndCell();

		// Return the resizing ID for the right-side of the given column.
		ImGuiID GetColumnResizeID(int column_n, int instance_no) {
			IM_ASSERT(column_n >= 0 && column_n < mTable.ColumnsCount);
			ImGuiID id = mTable.ID + 1 + (instance_no * mTable.ColumnsCount) + column_n;
			return id;
		}

		// Disable clipping then auto-fit, will take 2 frames
		// (we don't take a shortcut for unclipped columns to reduce inconsistencies when e.g. resizing multiple columns)
		void SetColumnWidthAutoSingle(int column_n) {
			// Single auto width uses auto-fit
			TableColumn* column = &mTable.Columns[column_n];
			if (!column->IsEnabled)
				return;
			column->CannotSkipItemsQueue = (1 << 0);
			mTable.AutoFitSingleColumn = (TableColumnIdx) column_n;
		}

		void BeginCell(int column_n);

		void BeginRow();

		void SetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1);

		// Note that the NoSortAscending/NoSortDescending flags are processed in TableSortSpecsSanitize(), and they may change/revert
		// the value of SortDirection. We could technically also do it here but it would be unnecessary and duplicate code.
		void SetColumnSortDirection(int column_n, ImGuiSortDirection sort_direction, bool append_to_sort_specs);

		// Calculate next sort direction that would be set after clicking the column
		// - If the PreferSortDescending flag is set, we will default to a Descending direction on the first click.
		// - Note that the PreferSortAscending flag is never checked, it is essentially the default and therefore a no-op.
		IM_STATIC_ASSERT(ImGuiSortDirection_None == 0 && ImGuiSortDirection_Ascending == 1 && ImGuiSortDirection_Descending == 2);
		ImGuiSortDirection GetColumnNextSortDirection(TableColumn* column);

		// 'width' = inner column width, without padding
		void SetColumnWidth(int column_n, float width);

		void UpdateColumnsWeightFromWidth();
	};

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	MainTable<MaxColumnCount>::MainTable(const std::vector<MainTableColumn>& pColumns, MainWindow* pMainWindow, MainTableFlags pFlags)
		: mMainWindow(pMainWindow), mColumns(pColumns), mFlags(pFlags) {

		IM_ASSERT(mColumns.size() < MaxColumnCount);

		mMainWindow->RegisterDrawStyleSubMenuHook([this] { DrawStyleSubMenu(); });

		// generate categories
		for (size_t i = 0; i < mColumns.size(); ++i) {
			const auto& column = mColumns[i];
			std::string cat = column.Category;
			size_t pos = 0;
			std::vector<size_t> token;
			while (true) {
				if ((pos = cat.find('.')) != std::string::npos) {
					std::string sub = cat.substr(0, pos);
					size_t dat = 0;
					std::from_chars(sub.data(), sub.data() + sub.size(), dat);
					token.push_back(dat);
					cat.erase(0, pos + 1);
				} else {
					size_t dat = 0;
					std::from_chars(cat.data(), cat.data() + cat.size(), dat);
					token.push_back(dat);
					break;
				}
			}

			if (token[0] == 0) {
				mCategories.Indices.emplace_back(i);
				continue;
			}

			Category* category = &mCategories;
			for (size_t tok : token) {
				category = &category->Categories[tok];
			}

			category->Indices.emplace_back(i);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::Draw() {
		ImGuiTableFlags tableFlags = ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Sortable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_PadOuterX | ImGuiTableFlags_ScrollY;

		if (getShowAlternatingBackground()) {
			tableFlags |= ImGuiTableFlags_RowBg;
		}

		ImVec2 outerSize(0.f, getMaxHeightActive() ? mMainWindow->GetMaxCursorPos() : 0.f);

		if (Begin(getTableId().c_str(), mColumns.size(), tableFlags, outerSize, 0,
				  getShowScrollbar() ? 0 : ImGuiWindowFlags_NoScrollbar)) {
			/**
			 * HEADER
			 */
			TableColumnIdx first = 0;
			for (int i = 0; i <= mTable.ColumnsCount; ++i) {
				if (mTable.EnabledMaskByDisplayOrder.test(i)) {
					first = mTable.DisplayOrderToIndex[i];
					break;
				}
			}

			// Column Setup
			for (int i = 0; i < mColumns.size(); ++i) {
				const auto& column = mColumns[i];

				ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_PreferSortDescending;
				if (first == i) {
					columnFlags |= ImGuiTableColumnFlags_IndentEnable;
				}
				if (!column.DefaultVisibility) {
					columnFlags |= ImGuiTableColumnFlags_DefaultHide;
				}
				SetupColumn(column.Name().c_str(), columnFlags, 0, column.UserId);
			}

			// freeze header
			SetupScrollFreeze(0, 1);

			NextRow(ImGuiTableRowFlags_Headers);

			for (const auto& column : mColumns) {
				if (NextColumn()) {
					ColumnHeader(column.Name().c_str(), false, column.Texture(), getHeaderAlignment(), column.Popup().c_str());
				}
			}

			/**
			 * SORTING
			 */
			if (ImGuiTableSortSpecs* sortSpecs = GetSortSpecs()) {
				if (sortSpecs->SpecsDirty) {
					mSortNeeded = true;
				}

				bool expected = true;
				if (mSortNeeded.compare_exchange_strong(expected, false)) {
					Sort(sortSpecs->Specs);

					sortSpecs->SpecsDirty = false;
				}
			}

			/**
			 * CONTENT
			 */
			mCurrentRow = 0;
			DrawRows(first);

			End();
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::DrawColumnSetupMenuIteratorFunc(Category pCategory, std::string pCatName) {
		for (const auto& index : pCategory.Indices) {
			MenuItemColumnVisibility(index);
		}

		for (const auto& [key, value] : pCategory.Categories) {
			std::string catName = pCatName;
			if (!catName.empty()) catName.append(".");
			catName.append(std::to_string(key));
			if (ImGui::BeginMenu(getCategoryName(catName))) {
				DrawColumnSetupMenuIteratorFunc(value, catName);

				ImGui::EndMenu();
			}
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::DrawColumnSetupMenu() {
		if (ImGui::BeginMenu(Localization::STranslate(ET_ColumnSetup).c_str())) {
			DrawColumnSetupSubMenu();

			ImGui::EndMenu();
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::DrawColumnSetupSubMenu() {
		if (getCustomColumnsFeatureActive()) {
			if (ImGui::Checkbox(Localization::STranslate(ET_ShowBasedOnMap).c_str(), &getCustomColumnsActive())) {
				if (getCustomColumnsActive()) {
					mSpecificColumnsUpdate = true;
					mSpecificColumnsActive = true;
				}
			}
		}

		DrawColumnSetupMenuIteratorFunc(mCategories);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetSpecificColumnSetup(const std::vector<size_t>& pNewColumns) {
		mSpecificColumnsUpdate = true;
		mSpecificColumnsActive = true;

		mSpecificColumnCache = pNewColumns;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::ResetSpecificColumnSetup() {
		mSpecificColumnsUpdate = true;
		mSpecificColumnsActive = false;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::DrawStyleSubMenu() {
		ImGui::Separator();
		ImGui::Checkbox(Localization::STranslate(ET_AlternatingRowBg).c_str(), &getShowAlternatingBackground());
		ImGui::Checkbox(Localization::STranslate(ET_HighlightHoveredRow).c_str(), &getHighlightHoveredRows());
		ImGui::Checkbox(Localization::STranslate(ET_SettingsShowHeaderText).c_str(), &getShowHeaderAsText());
		int& maxDisplayed = getMaxDisplayed();
		if (ImGui::InputInt(Localization::STranslate(ET_MaxDisplayed).c_str(), &maxDisplayed)) {
			if (maxDisplayed < 0) {
				maxDisplayed = 0;
			}
		}
		ImGuiEx::EnumCombo(Localization::STranslate(ET_HeaderAlignment).c_str(), getHeaderAlignment(), {Alignment::Left, Alignment::Center, Alignment::Right});
		ImGuiEx::EnumCombo(Localization::STranslate(ET_ColumnAlignment).c_str(), getAlignment(), {Alignment::Left, Alignment::Center, Alignment::Right});
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::NextRow(ImGuiTableRowFlags row_flags, float min_row_height) {
		if (!mTable.IsLayoutLocked)
			UpdateLayout();
		if (mTable.IsInsideRow)
			EndRow();

		mTable.LastRowFlags = mTable.RowFlags;
		mTable.RowFlags = row_flags;
		mTable.RowMinHeight = min_row_height;
		BeginRow();

		// We honor min_row_height requested by user, but cannot guarantee per-row maximum height,
		// because that would essentially require a unique clipping rectangle per-cell.
		mTable.RowPosY2 += mTable.CellPaddingY * 2.0f;
		mTable.RowPosY2 = ImMax(mTable.RowPosY2, mTable.RowPosY1 + min_row_height);

		// Disable output until user calls TableNextColumn()
		mTable.InnerWindow->SkipItems = true;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	bool MainTable<MaxColumnCount>::NextColumn() {
		if (mTable.IsInsideRow && mTable.CurrentColumn + 1 < mTable.ColumnsCount) {
			if (mTable.CurrentColumn != -1)
				EndCell();
			BeginCell(mTable.CurrentColumn + 1);
		} else {
			NextRow();
			BeginCell(0);
		}

		// Return whether the column is visible. User may choose to skip submitting items based on this return value,
		// however they shouldn't skip submitting for columns that may have the tallest contribution to row height.
		int column_n = mTable.CurrentColumn;
		return (mTable.RequestOutputMaskByIndex.test(column_n)) != 0;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::EndMaxHeightRow() {
		if (mCurrentRow < getMaxDisplayed()) {
			mMainWindow->SetMaxHeightCursorPos();
		}

		++mCurrentRow;
	}

	// Helper
	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	ImGuiTableFlags MainTable<MaxColumnCount>::TableFixFlags(ImGuiTableFlags flags, ImGuiWindow* outer_window) {
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

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::BeginInitMemory(int columns_count) {
		// Allocate single buffer for our arrays
		ImSpanAllocator<3> span_allocator;
		span_allocator.ReserveBytes(0, columns_count * sizeof(TableColumn));
		span_allocator.ReserveBytes(1, columns_count * sizeof(TableColumnIdx));
		span_allocator.ReserveBytes(2, columns_count * sizeof(ImGuiTableCellData));
		mTable.RawData = IM_ALLOC(span_allocator.GetArenaSizeInBytes());
		memset(mTable.RawData, 0, span_allocator.GetArenaSizeInBytes());
		span_allocator.SetArenaBasePtr(mTable.RawData);
		span_allocator.GetSpan(0, &mTable.Columns);
		span_allocator.GetSpan(1, &mTable.DisplayOrderToIndex);
		span_allocator.GetSpan(2, &mTable.RowCellData);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::ResetSettings() {
		mTable.IsInitializing = mTable.IsSettingsDirty = true;
		mTable.IsResetAllRequest = false;
		mTable.IsSettingsRequestLoad = false;              // Don't reload from ini
		mTable.SettingsLoadedFlags = ImGuiTableFlags_None; // Mark as nothing loaded so our initialized data becomes authoritative
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::LoadSettingsCustom() {
		mTable.IsSettingsRequestLoad = false;
		if (mTable.Flags & ImGuiTableFlags_NoSavedSettings)
			return;

		TableSettings& settings = getTableSettings();

		// Migration
		if (settings.Version == 0) {
			MigrateIniSettings();
			settings.Version = 1;
		}

		MigrateSettings();

		mTable.SettingsLoadedFlags = settings.SaveFlags;
		mTable.RefScale = settings.RefScale;

		// Serialize ImGuiTableSettings/ImGuiTableColumnSettings into ImGuiTable/ImGuiTableColumn
		ColumnBitMask display_order_mask;
		for (int idx = 0; idx < mTable.Columns.size(); ++idx) {
			TableColumn* column = &mTable.Columns[idx];

			// Preset userID, we need it for loading the settings
			column->UserID = mColumns[idx].UserId;

			const auto& column_settings = std::find(settings.Columns.begin(), settings.Columns.end(), column->UserID);
			if (column_settings == settings.Columns.end()) {
				continue;
			}

			if (settings.SaveFlags & ImGuiTableFlags_Resizable) {
				if (column_settings->IsStretch)
					column->StretchWeight = column_settings->WidthOrWeight;
				else
					column->WidthRequest = column_settings->WidthOrWeight;
				column->AutoFitQueue = 0x00;
			}
			if (settings.SaveFlags & ImGuiTableFlags_Reorderable)
				column->DisplayOrder = column_settings->DisplayOrder;
			else
				column->DisplayOrder = (TableColumnIdx) idx;
			if (column->DisplayOrder >= 0) display_order_mask.set(column->DisplayOrder);
			column->IsEnabled = column->IsEnabledNextFrame = column_settings->IsEnabled;
			column->SortOrder = column_settings->SortOrder;
			column->SortDirection = column_settings->SortDirection;
		}

		// Validate and fix invalid display order data
		if (display_order_mask.count() != settings.Columns.size())
			for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
				mTable.Columns[column_n].DisplayOrder = (TableColumnIdx) column_n;

		// Rebuild index
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
			mTable.DisplayOrderToIndex[mTable.Columns[column_n].DisplayOrder] = (TableColumnIdx) column_n;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::BeginApplyRequests() {
		// Handle resizing request
		// (We process this at the first TableBegin of the frame)
		// FIXME-TABLE: Contains columns if our work area doesn't allow for scrolling?
		if (mTable.InstanceCurrent == 0) {
			if (mTable.ResizedColumn != -1 && mTable.ResizedColumnNextWidth != FLT_MAX)
				SetColumnWidth(mTable.ResizedColumn, mTable.ResizedColumnNextWidth);
			mTable.LastResizedColumn = mTable.ResizedColumn;
			mTable.ResizedColumnNextWidth = FLT_MAX;
			mTable.ResizedColumn = -1;

			// Process auto-fit for single column, which is a special case for stretch columns and fixed columns with FixedSame policy.
			// FIXME-TABLE: Would be nice to redistribute available stretch space accordingly to other weights, instead of giving it all to siblings.
			if (mTable.AutoFitSingleColumn != -1) {
				SetColumnWidth(mTable.AutoFitSingleColumn, mTable.Columns[mTable.AutoFitSingleColumn].WidthAuto);
				mTable.AutoFitSingleColumn = -1;
			}
		}

		// Handle reordering request
		// Note: we don't clear ReorderColumn after handling the request.
		if (mTable.InstanceCurrent == 0) {
			if (mTable.HeldHeaderColumn == -1 && mTable.ReorderColumn != -1)
				mTable.ReorderColumn = -1;
			mTable.HeldHeaderColumn = -1;
			if (mTable.ReorderColumn != -1 && mTable.ReorderColumnDir != 0) {
				// We need to handle reordering across hidden columns.
				// In the configuration below, moving C to the right of E will lead to:
				//    ... C [D] E  --->  ... [D] E  C   (Column name/index)
				//    ... 2  3  4        ...  2  3  4   (Display order)
				const int reorder_dir = mTable.ReorderColumnDir;
				IM_ASSERT(reorder_dir == -1 || reorder_dir == +1);
				IM_ASSERT(mTable.Flags & ImGuiTableFlags_Reorderable);
				TableColumn* src_column = &mTable.Columns[mTable.ReorderColumn];
				TableColumn* dst_column = &mTable.Columns[(reorder_dir == -1) ? src_column->PrevEnabledColumn : src_column->NextEnabledColumn];
				IM_UNUSED(dst_column);
				const int src_order = src_column->DisplayOrder;
				const int dst_order = dst_column->DisplayOrder;
				src_column->DisplayOrder = (TableColumnIdx) dst_order;
				for (int order_n = src_order + reorder_dir; order_n != dst_order + reorder_dir; order_n += reorder_dir)
					mTable.Columns[mTable.DisplayOrderToIndex[order_n]].DisplayOrder -= (TableColumnIdx) reorder_dir;
				IM_ASSERT(dst_column->DisplayOrder == dst_order - reorder_dir);

				// Display order is stored in both columns->IndexDisplayOrder and table->DisplayOrder[],
				// rebuild the later from the former.
				for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
					mTable.DisplayOrderToIndex[mTable.Columns[column_n].DisplayOrder] = (TableColumnIdx) column_n;
				mTable.ReorderColumnDir = 0;
				mTable.IsSettingsDirty = true;
			}
		}

		// Handle display order reset request
		if (mTable.IsResetDisplayOrderRequest) {
			for (int n = 0; n < mTable.ColumnsCount; n++)
				mTable.DisplayOrderToIndex[n] = mTable.Columns[n].DisplayOrder = (TableColumnIdx) n;
			mTable.IsResetDisplayOrderRequest = false;
			mTable.IsSettingsDirty = true;
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	ImGuiTableSettings* MainTable<MaxColumnCount>::GetBoundSettings() {
		if (mTable.SettingsOffset != -1) {
			ImGuiContext& g = *GImGui;
			ImGuiTableSettings* settings = g.SettingsTables.ptr_from_offset(mTable.SettingsOffset);
			IM_ASSERT(settings->ID == mTable.ID);
			if (settings->ColumnsCountMax >= mTable.ColumnsCount)
				return settings; // OK
			settings->ID = 0;    // Invalidate storage, we won't fit because of a count change
		}
		return NULL;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::UpdateLayout() {
		ImGuiContext& g = *GImGui;
		IM_ASSERT(mTable.IsLayoutLocked == false);

		const ImGuiTableFlags table_sizing_policy = (mTable.Flags & ImGuiTableFlags_SizingMask_);
		mTable.IsDefaultDisplayOrder = true;
		mTable.ColumnsEnabledCount = 0;
		mTable.EnabledMaskByIndex = 0x00;
		mTable.EnabledMaskByDisplayOrder = 0x00;
		mTable.MinColumnWidth = ImMax(1.0f, g.Style.FramePadding.x * 1.0f); // g.Style.ColumnsMinSpacing; // FIXME-TABLE

		// [Part 1] Apply/lock Enabled and Order states. Calculate auto/ideal width for columns. Count fixed/stretch columns.
		// Process columns in their visible orders as we are building the Prev/Next indices.
		int count_fixed = 0;   // Number of columns that have fixed sizing policies
		int count_stretch = 0; // Number of columns that have stretch sizing policies
		int last_visible_column_idx = -1;
		bool has_auto_fit_request = false;
		bool has_resizable = false;
		float stretch_sum_width_auto = 0.0f;
		float fixed_max_width_auto = 0.0f;
		for (int order_n = 0; order_n < mTable.ColumnsCount; order_n++) {
			const int column_n = mTable.DisplayOrderToIndex[order_n];
			if (column_n != order_n)
				mTable.IsDefaultDisplayOrder = false;
			TableColumn* column = &mTable.Columns[column_n];

			// Clear column setup if not submitted by user. Currently we make it mandatory to call TableSetupColumn() every frame.
			// It would easily work without but we're not ready to guarantee it since e.g. names need resubmission anyway.
			// We take a slight shortcut but in theory we could be calling TableSetupColumn() here with dummy values, it should yield the same effect.
			if (mTable.DeclColumnsCount <= column_n) {
				SetupColumnFlags(column, ImGuiTableColumnFlags_None);
				column->NameOffset = -1;
				column->UserID = 0;
				column->InitStretchWeightOrWidth = -1.0f;
			}

			// Update Enabled state, mark settings/sortspecs dirty
			if (!(mTable.Flags & ImGuiTableFlags_Hideable) || (column->Flags & ImGuiTableColumnFlags_NoHide))
				column->IsEnabledNextFrame = true;
			if (column->IsEnabled != column->IsEnabledNextFrame) {
				column->IsEnabled = column->IsEnabledNextFrame;
				mTable.IsSettingsDirty = true;
				if (!column->IsEnabled && column->SortOrder != -1)
					mTable.IsSortSpecsDirty = true;
			}
			if (column->SortOrder > 0 && !(mTable.Flags & ImGuiTableFlags_SortMulti))
				mTable.IsSortSpecsDirty = true;

			// Auto-fit unsized columns
			const bool start_auto_fit = (column->Flags & ImGuiTableColumnFlags_WidthFixed) ? (column->WidthRequest < 0.0f) : (column->StretchWeight < 0.0f);
			if (start_auto_fit)
				column->AutoFitQueue = column->CannotSkipItemsQueue = (1 << 3) - 1; // Fit for three frames

			if (!column->IsEnabled) {
				column->IndexWithinEnabledSet = -1;
				continue;
			}

			// Mark as enabled and link to previous/next enabled column
			column->PrevEnabledColumn = (TableColumnIdx) last_visible_column_idx;
			column->NextEnabledColumn = -1;
			if (last_visible_column_idx != -1)
				mTable.Columns[last_visible_column_idx].NextEnabledColumn = (TableColumnIdx) column_n;
			column->IndexWithinEnabledSet = mTable.ColumnsEnabledCount++;
			mTable.EnabledMaskByIndex.set(column_n);
			mTable.EnabledMaskByDisplayOrder.set(column->DisplayOrder);
			last_visible_column_idx = column_n;
			IM_ASSERT(column->IndexWithinEnabledSet <= column->DisplayOrder);

			// Calculate ideal/auto column width (that's the width required for all contents to be visible without clipping)
			// Combine width from regular rows + width from headers unless requested not to.
			if (!column->IsPreserveWidthAuto)
				column->WidthAuto = GetColumnWidthAuto(column);

			// Non-resizable columns keep their requested width (apply user value regardless of IsPreserveWidthAuto)
			const bool column_is_resizable = (column->Flags & ImGuiTableColumnFlags_NoResize) == 0;
			if (column_is_resizable)
				has_resizable = true;
			if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f && !column_is_resizable)
				column->WidthAuto = column->InitStretchWeightOrWidth;

			if (column->AutoFitQueue != 0x00)
				has_auto_fit_request = true;
			if (column->Flags & ImGuiTableColumnFlags_WidthStretch) {
				stretch_sum_width_auto += column->WidthAuto;
				count_stretch++;
			} else {
				fixed_max_width_auto = ImMax(fixed_max_width_auto, column->WidthAuto);
				count_fixed++;
			}
		}
		if ((mTable.Flags & ImGuiTableFlags_Sortable) && mTable.SortSpecsCount == 0 && !(mTable.Flags & ImGuiTableFlags_SortTristate))
			mTable.IsSortSpecsDirty = true;
		mTable.RightMostEnabledColumn = (TableColumnIdx) last_visible_column_idx;
		IM_ASSERT(mTable.RightMostEnabledColumn >= 0);

		// [Part 2] Disable child window clipping while fitting columns. This is not strictly necessary but makes it possible
		// to avoid the column fitting having to wait until the first visible frame of the child container (may or not be a good thing).
		// FIXME-TABLE: for always auto-resizing columns may not want to do that all the time.
		if (has_auto_fit_request && mTable.OuterWindow != mTable.InnerWindow)
			mTable.InnerWindow->SkipItems = false;
		if (has_auto_fit_request)
			mTable.IsSettingsDirty = true;

		// [Part 3] Fix column flags and record a few extra information.
		float sum_width_requests = 0.0f;  // Sum of all width for fixed and auto-resize columns, excluding width contributed by Stretch columns but including spacing/padding.
		float stretch_sum_weights = 0.0f; // Sum of all weights for stretch columns.
		mTable.LeftMostStretchedColumn = mTable.RightMostStretchedColumn = -1;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			if (!mTable.EnabledMaskByIndex.test(column_n))
				continue;
			TableColumn* column = &mTable.Columns[column_n];

			const bool column_is_resizable = (column->Flags & ImGuiTableColumnFlags_NoResize) == 0;
			if (column->Flags & ImGuiTableColumnFlags_WidthFixed) {
				// Apply same widths policy
				float width_auto = column->WidthAuto;
				if (table_sizing_policy == ImGuiTableFlags_SizingFixedSame && (column->AutoFitQueue != 0x00 || !column_is_resizable))
					width_auto = fixed_max_width_auto;

				// Apply automatic width
				// Latch initial size for fixed columns and update it constantly for auto-resizing column (unless clipped!)
				if (column->AutoFitQueue != 0x00)
					column->WidthRequest = width_auto;
				else if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && !column_is_resizable && mTable.RequestOutputMaskByIndex.test(column_n))
					column->WidthRequest = width_auto;

				// FIXME-TABLE: Increase minimum size during init frame to avoid biasing auto-fitting widgets
				// (e.g. TextWrapped) too much. Otherwise what tends to happen is that TextWrapped would output a very
				// large height (= first frame scrollbar display very off + clipper would skip lots of items).
				// This is merely making the side-effect less extreme, but doesn't properly fixes it.
				// FIXME: Move this to ->WidthGiven to avoid temporary lossyless?
				// FIXME: This break IsPreserveWidthAuto from not flickering if the stored WidthAuto was smaller.
				if (column->AutoFitQueue > 0x01 && mTable.IsInitializing && !column->IsPreserveWidthAuto)
					column->WidthRequest = ImMax(column->WidthRequest, mTable.MinColumnWidth * 4.0f); // FIXME-TABLE: Another constant/scale?
				sum_width_requests += column->WidthRequest;
			} else {
				// Initialize stretch weight
				if (column->AutoFitQueue != 0x00 || column->StretchWeight < 0.0f || !column_is_resizable) {
					if (column->InitStretchWeightOrWidth > 0.0f)
						column->StretchWeight = column->InitStretchWeightOrWidth;
					else if (table_sizing_policy == ImGuiTableFlags_SizingStretchProp)
						column->StretchWeight = (column->WidthAuto / stretch_sum_width_auto) * count_stretch;
					else
						column->StretchWeight = 1.0f;
				}

				stretch_sum_weights += column->StretchWeight;
				if (mTable.LeftMostStretchedColumn == -1 || mTable.Columns[mTable.LeftMostStretchedColumn].DisplayOrder > column->DisplayOrder)
					mTable.LeftMostStretchedColumn = (TableColumnIdx) column_n;
				if (mTable.RightMostStretchedColumn == -1 || mTable.Columns[mTable.RightMostStretchedColumn].DisplayOrder < column->DisplayOrder)
					mTable.RightMostStretchedColumn = (TableColumnIdx) column_n;
			}
			column->IsPreserveWidthAuto = false;
			sum_width_requests += mTable.CellPaddingX * 2.0f;
		}
		mTable.ColumnsEnabledFixedCount = (TableColumnIdx) count_fixed;

		// [Part 4] Apply final widths based on requested widths
		const ImRect work_rect = mTable.WorkRect;
		const float width_spacings = (mTable.OuterPaddingX * 2.0f) + (mTable.CellSpacingX1 + mTable.CellSpacingX2) * (mTable.ColumnsEnabledCount - 1);
		const float width_avail = ((mTable.Flags & ImGuiTableFlags_ScrollX) && mTable.InnerWidth == 0.0f) ? mTable.InnerClipRect.GetWidth() : work_rect.GetWidth();
		const float width_avail_for_stretched_columns = width_avail - width_spacings - sum_width_requests;
		float width_remaining_for_stretched_columns = width_avail_for_stretched_columns;
		mTable.ColumnsGivenWidth = width_spacings + (mTable.CellPaddingX * 2.0f) * mTable.ColumnsEnabledCount;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			if (!mTable.EnabledMaskByIndex.test(column_n))
				continue;
			TableColumn* column = &mTable.Columns[column_n];

			// Allocate width for stretched/weighted columns (StretchWeight gets converted into WidthRequest)
			if (column->Flags & ImGuiTableColumnFlags_WidthStretch) {
				float weight_ratio = column->StretchWeight / stretch_sum_weights;
				column->WidthRequest = IM_FLOOR(ImMax(width_avail_for_stretched_columns * weight_ratio, mTable.MinColumnWidth) + 0.01f);
				width_remaining_for_stretched_columns -= column->WidthRequest;
			}

			// [Resize Rule 1] The right-most Visible column is not resizable if there is at least one Stretch column
			// See additional comments in TableSetColumnWidth().
			if (column->NextEnabledColumn == -1 && mTable.LeftMostStretchedColumn != -1)
				column->Flags |= ImGuiTableColumnFlags_NoDirectResize_;

			// Assign final width, record width in case we will need to shrink
			column->WidthGiven = ImFloor(ImMax(column->WidthRequest, mTable.MinColumnWidth));
			mTable.ColumnsGivenWidth += column->WidthGiven;
		}

		// [Part 5] Redistribute stretch remainder width due to rounding (remainder width is < 1.0f * number of Stretch column).
		// Using right-to-left distribution (more likely to match resizing cursor).
		if (width_remaining_for_stretched_columns >= 1.0f && !(mTable.Flags & ImGuiTableFlags_PreciseWidths))
			for (int order_n = mTable.ColumnsCount - 1; stretch_sum_weights > 0.0f && width_remaining_for_stretched_columns >= 1.0f && order_n >= 0; order_n--) {
				if (!mTable.EnabledMaskByDisplayOrder.test(order_n))
					continue;
				TableColumn* column = &mTable.Columns[mTable.DisplayOrderToIndex[order_n]];
				if (!(column->Flags & ImGuiTableColumnFlags_WidthStretch))
					continue;
				column->WidthRequest += 1.0f;
				column->WidthGiven += 1.0f;
				width_remaining_for_stretched_columns -= 1.0f;
			}

		mTable.HoveredColumnBody = -1;
		mTable.HoveredColumnBorder = -1;
		const ImRect mouse_hit_rect(mTable.OuterRect.Min.x, mTable.OuterRect.Min.y, mTable.OuterRect.Max.x, ImMax(mTable.OuterRect.Max.y, mTable.OuterRect.Min.y + mTable.LastOuterHeight));
		const bool is_hovering_table = ImGui::ItemHoverable(mouse_hit_rect, 0);

		// [Part 6] Setup final position, offset, skip/clip states and clipping rectangles, detect hovered column
		// Process columns in their visible orders as we are comparing the visible order and adjusting host_clip_rect while looping.
		int visible_n = 0;
		bool offset_x_frozen = (mTable.FreezeColumnsCount > 0);
		float offset_x = ((mTable.FreezeColumnsCount > 0) ? mTable.OuterRect.Min.x : work_rect.Min.x) + mTable.OuterPaddingX - mTable.CellSpacingX1;
		ImRect host_clip_rect = mTable.InnerClipRect;
		//host_clip_rect.Max.x += mTable.CellPaddingX + mTable.CellSpacingX2;
		mTable.VisibleMaskByIndex = 0x00;
		mTable.RequestOutputMaskByIndex = 0x00;
		for (int order_n = 0; order_n < mTable.ColumnsCount; order_n++) {
			const int column_n = mTable.DisplayOrderToIndex[order_n];
			TableColumn* column = &mTable.Columns[column_n];

			column->NavLayerCurrent = (ImS8) ((mTable.FreezeRowsCount > 0 || column_n < mTable.FreezeColumnsCount) ? ImGuiNavLayer_Menu : ImGuiNavLayer_Main);

			if (offset_x_frozen && mTable.FreezeColumnsCount == visible_n) {
				offset_x += work_rect.Min.x - mTable.OuterRect.Min.x;
				offset_x_frozen = false;
			}

			// Clear status flags
			column->Flags &= ~ImGuiTableColumnFlags_StatusMask_;

			if (!mTable.EnabledMaskByDisplayOrder.test(order_n)) {
				// Hidden column: clear a few fields and we are done with it for the remainder of the function.
				// We set a zero-width clip rect but set Min.y/Max.y properly to not interfere with the clipper.
				column->MinX = column->MaxX = column->WorkMinX = column->ClipRect.Min.x = column->ClipRect.Max.x = offset_x;
				column->WidthGiven = 0.0f;
				column->ClipRect.Min.y = work_rect.Min.y;
				column->ClipRect.Max.y = FLT_MAX;
				column->ClipRect.ClipWithFull(host_clip_rect);
				column->IsVisibleX = column->IsVisibleY = column->IsRequestOutput = false;
				column->IsSkipItems = true;
				column->ItemWidth = 1.0f;
				continue;
			}

			// Detect hovered column
			if (is_hovering_table && g.IO.MousePos.x >= column->ClipRect.Min.x && g.IO.MousePos.x < column->ClipRect.Max.x)
				mTable.HoveredColumnBody = (TableColumnIdx) column_n;

			// Lock start position
			column->MinX = offset_x;

			// Lock width based on start position and minimum/maximum width for this position
			float max_width = GetMaxColumnWidth(column_n);
			column->WidthGiven = ImMin(column->WidthGiven, max_width);
			column->WidthGiven = ImMax(column->WidthGiven, ImMin(column->WidthRequest, mTable.MinColumnWidth));
			column->MaxX = offset_x + column->WidthGiven + mTable.CellSpacingX1 + mTable.CellSpacingX2 + mTable.CellPaddingX * 2.0f;

			// Lock other positions
			// - ClipRect.Min.x: Because merging draw commands doesn't compare min boundaries, we make ClipRect.Min.x match left bounds to be consistent regardless of merging.
			// - ClipRect.Max.x: using WorkMaxX instead of MaxX (aka including padding) makes things more consistent when resizing down, tho slightly detrimental to visibility in very-small column.
			// - ClipRect.Max.x: using MaxX makes it easier for header to receive hover highlight with no discontinuity and display sorting arrow.
			// - FIXME-TABLE: We want equal width columns to have equal (ClipRect.Max.x - WorkMinX) width, which means ClipRect.max.x cannot stray off host_clip_rect.Max.x else right-most column may appear shorter.
			column->WorkMinX = column->MinX + mTable.CellPaddingX + mTable.CellSpacingX1;
			column->WorkMaxX = column->MaxX - mTable.CellPaddingX - mTable.CellSpacingX2; // Expected max
			column->ItemWidth = ImFloor(column->WidthGiven * 0.65f);
			column->ClipRect.Min.x = column->MinX;
			column->ClipRect.Min.y = work_rect.Min.y;
			column->ClipRect.Max.x = column->MaxX; //column->WorkMaxX;
			column->ClipRect.Max.y = FLT_MAX;
			column->ClipRect.ClipWithFull(host_clip_rect);

			// Mark column as Clipped (not in sight)
			// Note that scrolling tables (where inner_window != outer_window) handle Y clipped earlier in BeginTable() so IsVisibleY really only applies to non-scrolling tables.
			// FIXME-TABLE: Because InnerClipRect.Max.y is conservatively ==outer_window->ClipRect.Max.y, we never can mark columns _Above_ the scroll line as not IsVisibleY.
			// Taking advantage of LastOuterHeight would yield good results there...
			// FIXME-TABLE: Y clipping is disabled because it effectively means not submitting will reduce contents width which is fed to outer_window->DC.CursorMaxPos.x,
			// and this may be used (e.g. typically by outer_window using AlwaysAutoResize or outer_window's horizontal scrollbar, but could be something else).
			// Possible solution to preserve last known content width for clipped column. Test 'table_reported_size' fails when enabling Y clipping and window is resized small.
			column->IsVisibleX = (column->ClipRect.Max.x > column->ClipRect.Min.x);
			column->IsVisibleY = true;                  // (column->ClipRect.Max.y > column->ClipRect.Min.y);
			const bool is_visible = column->IsVisibleX; //&& column->IsVisibleY;
			if (is_visible)
				mTable.VisibleMaskByIndex.set(column_n);

			// Mark column as requesting output from user. Note that fixed + non-resizable sets are auto-fitting at all times and therefore always request output.
			column->IsRequestOutput = is_visible || column->AutoFitQueue != 0 || column->CannotSkipItemsQueue != 0;
			if (column->IsRequestOutput)
				mTable.RequestOutputMaskByIndex.set(column_n);

			// Mark column as SkipItems (ignoring all items/layout)
			column->IsSkipItems = !column->IsEnabled || mTable.HostSkipItems;
			if (column->IsSkipItems)
				IM_ASSERT(!is_visible);

			// Update status flags
			column->Flags |= ImGuiTableColumnFlags_IsEnabled;
			if (is_visible)
				column->Flags |= ImGuiTableColumnFlags_IsVisible;
			if (column->SortOrder != -1)
				column->Flags |= ImGuiTableColumnFlags_IsSorted;
			if (mTable.HoveredColumnBody == column_n)
				column->Flags |= ImGuiTableColumnFlags_IsHovered;

			// Alignment
			// FIXME-TABLE: This align based on the whole column width, not per-cell, and therefore isn't useful in
			// many cases (to be able to honor this we might be able to store a log of cells width, per row, for
			// visible rows, but nav/programmatic scroll would have visible artifacts.)
			//if (column->Flags & ImGuiTableColumnFlags_AlignRight)
			//    column->WorkMinX = ImMax(column->WorkMinX, column->MaxX - column->ContentWidthRowsUnfrozen);
			//else if (column->Flags & ImGuiTableColumnFlags_AlignCenter)
			//    column->WorkMinX = ImLerp(column->WorkMinX, ImMax(column->StartX, column->MaxX - column->ContentWidthRowsUnfrozen), 0.5f);

			// Reset content width variables
			column->ContentMaxXFrozen = column->ContentMaxXUnfrozen = column->WorkMinX;
			column->ContentMaxXHeadersUsed = column->ContentMaxXHeadersIdeal = column->WorkMinX;

			// Don't decrement auto-fit counters until container window got a chance to submit its items
			if (mTable.HostSkipItems == false) {
				column->AutoFitQueue >>= 1;
				column->CannotSkipItemsQueue >>= 1;
			}

			if (visible_n < mTable.FreezeColumnsCount)
				host_clip_rect.Min.x = ImClamp(column->MaxX + TABLE_BORDER_SIZE, host_clip_rect.Min.x, host_clip_rect.Max.x);

			offset_x += column->WidthGiven + mTable.CellSpacingX1 + mTable.CellSpacingX2 + mTable.CellPaddingX * 2.0f;
			visible_n++;
		}

		// [Part 7] Detect/store when we are hovering the unused space after the right-most column (so e.g. context menus can react on it)
		// Clear Resizable flag if none of our column are actually resizable (either via an explicit _NoResize flag, either
		// because of using _WidthAuto/_WidthStretch). This will hide the resizing option from the context menu.
		const float unused_x1 = ImMax(mTable.WorkRect.Min.x, mTable.Columns[mTable.RightMostEnabledColumn].ClipRect.Max.x);
		if (is_hovering_table && mTable.HoveredColumnBody == -1) {
			if (g.IO.MousePos.x >= unused_x1)
				mTable.HoveredColumnBody = (TableColumnIdx) mTable.ColumnsCount;
		}
		if (has_resizable == false && (mTable.Flags & ImGuiTableFlags_Resizable))
			mTable.Flags &= ~ImGuiTableFlags_Resizable;

		// [Part 8] Lock actual OuterRect/WorkRect right-most position.
		// This is done late to handle the case of fixed-columns tables not claiming more widths that they need.
		// Because of this we are careful with uses of WorkRect and InnerClipRect before this point.
		if (mTable.RightMostStretchedColumn != -1)
			mTable.Flags &= ~ImGuiTableFlags_NoHostExtendX;
		if (mTable.Flags & ImGuiTableFlags_NoHostExtendX) {
			mTable.OuterRect.Max.x = mTable.WorkRect.Max.x = unused_x1;
			mTable.InnerClipRect.Max.x = ImMin(mTable.InnerClipRect.Max.x, unused_x1);
		}
		mTable.InnerWindow->ParentWorkRect = mTable.WorkRect;
		mTable.BorderX1 = mTable.InnerClipRect.Min.x; // +((mTable.Flags & ImGuiTableFlags_BordersOuter) ? 0.0f : -1.0f);
		mTable.BorderX2 = mTable.InnerClipRect.Max.x; // +((mTable.Flags & ImGuiTableFlags_BordersOuter) ? 0.0f : +1.0f);

		// [Part 9] Allocate draw channels and setup background cliprect
		SetupDrawChannels();

		// [Part 10] Hit testing on borders
		if (mTable.Flags & ImGuiTableFlags_Resizable)
			UpdateBorders();
		mTable.LastFirstRowHeight = 0.0f;
		mTable.IsLayoutLocked = true;
		mTable.IsUsingHeaders = false;

		// [Part 11] Context menu
		// FIXME: Context menu disabled -knox
		// if (mTable.IsContextPopupOpen && mTable.InstanceCurrent == mTable.InstanceInteracted)
		// {
		//     const ImGuiID context_menu_id = ImHashStr("##ContextMenu", 0, mTable.ID);
		//     if (BeginPopupEx(context_menu_id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
		//     {
		//         TableDrawContextMenu(table);
		//         EndPopup();
		//     }
		//     else
		//     {
		//         mTable.IsContextPopupOpen = false;
		//     }
		// }

		// [Part 13] Sanitize and build sort specs before we have a change to use them for display.
		// This path will only be exercised when sort specs are modified before header rows (e.g. init or visibility change)
		if (mTable.IsSortSpecsDirty && (mTable.Flags & ImGuiTableFlags_Sortable))
			SortSpecsBuild();

		// Initial state
		ImGuiWindow* inner_window = mTable.InnerWindow;
		if (mTable.Flags & ImGuiTableFlags_NoClip)
			mTable.DrawSplitter.SetCurrentChannel(inner_window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
		else
			inner_window->DrawList->PushClipRect(inner_window->ClipRect.Min, inner_window->ClipRect.Max, false);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SortSpecsBuild() {
		IM_ASSERT(mTable.IsSortSpecsDirty);
		SortSpecsSanitize();

		// Write output
		mTable.SortSpecsMulti.resize(mTable.SortSpecsCount <= 1 ? 0 : mTable.SortSpecsCount);
		ImGuiTableColumnSortSpecs* sort_specs = (mTable.SortSpecsCount == 0) ? NULL : (mTable.SortSpecsCount == 1) ? &mTable.SortSpecsSingle
																												   : mTable.SortSpecsMulti.Data;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			TableColumn* column = &mTable.Columns[column_n];
			if (column->SortOrder == -1)
				continue;
			IM_ASSERT(column->SortOrder < mTable.SortSpecsCount);
			ImGuiTableColumnSortSpecs* sort_spec = &sort_specs[column->SortOrder];
			sort_spec->ColumnUserID = column->UserID;
			sort_spec->ColumnIndex = (TableColumnIdx) column_n;
			sort_spec->SortOrder = (TableColumnIdx) column->SortOrder;
			sort_spec->SortDirection = column->SortDirection;
		}
		mTable.SortSpecs.Specs = sort_specs;
		mTable.SortSpecs.SpecsCount = mTable.SortSpecsCount;
		mTable.SortSpecs.SpecsDirty = true; // Mark as dirty for user
		mTable.IsSortSpecsDirty = false;    // Mark as not dirty for us
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::TableSetupColumnFlags(TableColumn* column, ImGuiTableColumnFlags flags_in) {
		ImGuiTableColumnFlags flags = flags_in;

		// Sizing Policy
		if ((flags & ImGuiTableColumnFlags_WidthMask_) == 0) {
			const ImGuiTableFlags table_sizing_policy = (mTable.Flags & ImGuiTableFlags_SizingMask_);
			if (table_sizing_policy == ImGuiTableFlags_SizingFixedFit || table_sizing_policy == ImGuiTableFlags_SizingFixedSame)
				flags |= ImGuiTableColumnFlags_WidthFixed;
			else
				flags |= ImGuiTableColumnFlags_WidthStretch;
		} else {
			IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_WidthMask_)); // Check that only 1 of each set is used.
		}

		// Resize
		if ((mTable.Flags & ImGuiTableFlags_Resizable) == 0)
			flags |= ImGuiTableColumnFlags_NoResize;

		// Sorting
		if ((flags & ImGuiTableColumnFlags_NoSortAscending) && (flags & ImGuiTableColumnFlags_NoSortDescending))
			flags |= ImGuiTableColumnFlags_NoSort;

		// Indentation
		if ((flags & ImGuiTableColumnFlags_IndentMask_) == 0)
			flags |= (mTable.Columns.index_from_ptr(column) == 0) ? ImGuiTableColumnFlags_IndentEnable : ImGuiTableColumnFlags_IndentDisable;

		// Alignment
		//if ((flags & ImGuiTableColumnFlags_AlignMask_) == 0)
		//    flags |= ImGuiTableColumnFlags_AlignCenter;
		//IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_AlignMask_)); // Check that only 1 of each set is used.

		// Preserve status flags
		column->Flags = flags | (column->Flags & ImGuiTableColumnFlags_StatusMask_);

		// Build an ordered list of available sort directions
		column->SortDirectionsAvailCount = column->SortDirectionsAvailMask = column->SortDirectionsAvailList = 0;
		if (mTable.Flags & ImGuiTableFlags_Sortable) {
			int count = 0, mask = 0, list = 0;
			if ((flags & ImGuiTableColumnFlags_PreferSortAscending) != 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) {
				mask |= 1 << ImGuiSortDirection_Ascending;
				list |= ImGuiSortDirection_Ascending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortDescending) != 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) {
				mask |= 1 << ImGuiSortDirection_Descending;
				list |= ImGuiSortDirection_Descending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortAscending) == 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) {
				mask |= 1 << ImGuiSortDirection_Ascending;
				list |= ImGuiSortDirection_Ascending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortDescending) == 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) {
				mask |= 1 << ImGuiSortDirection_Descending;
				list |= ImGuiSortDirection_Descending << (count << 1);
				count++;
			}
			if ((mTable.Flags & ImGuiTableFlags_SortTristate) || count == 0) {
				mask |= 1 << ImGuiSortDirection_None;
				count++;
			}
			column->SortDirectionsAvailList = (ImU8) list;
			column->SortDirectionsAvailMask = (ImU8) mask;
			column->SortDirectionsAvailCount = (ImU8) count;
			FixColumnSortDirection(column);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::EndRow() {
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		IM_ASSERT(window == mTable.InnerWindow);
		IM_ASSERT(mTable.IsInsideRow);

		if (mTable.CurrentColumn != -1)
			EndCell();

		// Position cursor at the bottom of our row so it can be used for e.g. clipping calculation. However it is
		// likely that the next call to TableBeginCell() will reposition the cursor to take account of vertical padding.
		window->DC.CursorPos.y = mTable.RowPosY2;

		// Row background fill
		const float bg_y1 = mTable.RowPosY1;
		const float bg_y2 = mTable.RowPosY2;
		const bool unfreeze_rows_actual = (mTable.CurrentRow + 1 == mTable.FreezeRowsCount);
		const bool unfreeze_rows_request = (mTable.CurrentRow + 1 == mTable.FreezeRowsRequest);
		if (mTable.CurrentRow == 0)
			mTable.LastFirstRowHeight = bg_y2 - bg_y1;

		const bool is_visible = (bg_y2 >= mTable.InnerClipRect.Min.y && bg_y1 <= mTable.InnerClipRect.Max.y);
		if (is_visible) {
			// Decide of background color for the row
			ImU32 bg_col0 = 0;
			ImU32 bg_col1 = 0;
			if (mTable.RowBgColor[0] != IM_COL32_DISABLE)
				bg_col0 = mTable.RowBgColor[0];
			else if (mTable.Flags & ImGuiTableFlags_RowBg)
				bg_col0 = ImGui::GetColorU32((mTable.RowBgColorCounter & 1) ? ImGuiCol_TableRowBgAlt : ImGuiCol_TableRowBg);
			if (mTable.RowBgColor[1] != IM_COL32_DISABLE)
				bg_col1 = mTable.RowBgColor[1];

			// KNOX: override bg color for hovered rows
			ImRect row_rect(mTable.WorkRect.Min.x, bg_y1, mTable.WorkRect.Max.x, bg_y2);
			row_rect.ClipWith(mTable.BgClipRect);

			// [[ DEBUG ]]
			// ImGui::GetCurrentWindow()->DrawList->AddRect(row_rect.Min, row_rect.Max, 0xff0000ff);

			if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(row_rect.Min, row_rect.Max, false) && (mTable.CurrentRow > 0 && mTable.IsUsingHeaders)) {
				mTable.CurrentHoveredRow = mTable.CurrentRow;

				if (getHighlightHoveredRows()) {
					bg_col1 = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
				}
			} else if (mTable.CurrentHoveredRow == mTable.CurrentRow) {
				mTable.CurrentHoveredRow = -1;
			}


			// Decide of top border color
			ImU32 border_col = 0;
			const float border_size = TABLE_BORDER_SIZE;
			if (mTable.CurrentRow > 0 || mTable.InnerWindow == mTable.OuterWindow)
				if (mTable.Flags & ImGuiTableFlags_BordersInnerH)
					border_col = (mTable.LastRowFlags & ImGuiTableRowFlags_Headers) ? mTable.BorderColorStrong : mTable.BorderColorLight;

			const bool draw_cell_bg_color = mTable.RowCellDataCurrent >= 0;
			const bool draw_strong_bottom_border = unfreeze_rows_actual;
			if ((bg_col0 | bg_col1 | border_col) != 0 || draw_strong_bottom_border || draw_cell_bg_color) {
				// In theory we could call SetWindowClipRectBeforeSetChannel() but since we know TableEndRow() is
				// always followed by a change of clipping rectangle we perform the smallest overwrite possible here.
				if ((mTable.Flags & ImGuiTableFlags_NoClip) == 0)
					window->DrawList->_CmdHeader.ClipRect = mTable.Bg0ClipRectForDrawCmd.ToVec4();
				mTable.DrawSplitter.SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_BG0);
			}

			// Draw row background
			// We soft/cpu clip this so all backgrounds and borders can share the same clipping rectangle
			if (bg_col0 || bg_col1) {
				if (bg_col0 != 0 && row_rect.Min.y < row_rect.Max.y)
					window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col0);
				if (bg_col1 != 0 && row_rect.Min.y < row_rect.Max.y)
					window->DrawList->AddRectFilled(row_rect.Min, row_rect.Max, bg_col1);
			}

			// Draw cell background color
			if (draw_cell_bg_color) {
				ImGuiTableCellData* cell_data_end = &mTable.RowCellData[mTable.RowCellDataCurrent];
				for (ImGuiTableCellData* cell_data = &mTable.RowCellData[0]; cell_data <= cell_data_end; cell_data++) {
					const TableColumn* column = &mTable.Columns[cell_data->Column];
					ImRect cell_bg_rect = GetCellBgRect(cell_data->Column);
					cell_bg_rect.ClipWith(mTable.BgClipRect);
					cell_bg_rect.Min.x = ImMax(cell_bg_rect.Min.x, column->ClipRect.Min.x); // So that first column after frozen one gets clipped
					cell_bg_rect.Max.x = ImMin(cell_bg_rect.Max.x, column->MaxX);
					window->DrawList->AddRectFilled(cell_bg_rect.Min, cell_bg_rect.Max, cell_data->BgColor);
				}
			}

			// Draw top border
			if (border_col && bg_y1 >= mTable.BgClipRect.Min.y && bg_y1 < mTable.BgClipRect.Max.y)
				window->DrawList->AddLine(ImVec2(mTable.BorderX1, bg_y1), ImVec2(mTable.BorderX2, bg_y1), border_col, border_size);

			// Draw bottom border at the row unfreezing mark (always strong)
			if (draw_strong_bottom_border && bg_y2 >= mTable.BgClipRect.Min.y && bg_y2 < mTable.BgClipRect.Max.y)
				window->DrawList->AddLine(ImVec2(mTable.BorderX1, bg_y2), ImVec2(mTable.BorderX2, bg_y2), mTable.BorderColorStrong, border_size);
		}

		// End frozen rows (when we are past the last frozen row line, teleport cursor and alter clipping rectangle)
		// We need to do that in TableEndRow() instead of TableBeginRow() so the list clipper can mark end of row and
		// get the new cursor position.
		if (unfreeze_rows_request)
			for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
				TableColumn* column = &mTable.Columns[column_n];
				column->NavLayerCurrent = (ImS8) ((column_n < mTable.FreezeColumnsCount) ? ImGuiNavLayer_Menu : ImGuiNavLayer_Main);
			}
		if (unfreeze_rows_actual) {
			IM_ASSERT(mTable.IsUnfrozenRows == false);
			mTable.IsUnfrozenRows = true;

			// BgClipRect starts as mTable.InnerClipRect, reduce it now and make BgClipRectForDrawCmd == BgClipRect
			float y0 = ImMax(mTable.RowPosY2 + 1, window->InnerClipRect.Min.y);
			mTable.BgClipRect.Min.y = mTable.Bg2ClipRectForDrawCmd.Min.y = ImMin(y0, window->InnerClipRect.Max.y);
			mTable.BgClipRect.Max.y = mTable.Bg2ClipRectForDrawCmd.Max.y = window->InnerClipRect.Max.y;
			mTable.Bg2DrawChannelCurrent = mTable.Bg2DrawChannelUnfrozen;
			IM_ASSERT(mTable.Bg2ClipRectForDrawCmd.Min.y <= mTable.Bg2ClipRectForDrawCmd.Max.y);

			float row_height = mTable.RowPosY2 - mTable.RowPosY1;
			mTable.RowPosY2 = window->DC.CursorPos.y = mTable.WorkRect.Min.y + mTable.RowPosY2 - mTable.OuterRect.Min.y;
			mTable.RowPosY1 = mTable.RowPosY2 - row_height;
			for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
				TableColumn* column = &mTable.Columns[column_n];
				column->DrawChannelCurrent = column->DrawChannelUnfrozen;
				column->ClipRect.Min.y = mTable.Bg2ClipRectForDrawCmd.Min.y;
			}

			// Update cliprect ahead of TableBeginCell() so clipper can access to new ClipRect->Min.y
			ImGui::SetWindowClipRectBeforeSetChannel(window, mTable.Columns[0].ClipRect);
			mTable.DrawSplitter.SetCurrentChannel(window->DrawList, mTable.Columns[0].DrawChannelCurrent);
		}

		if (!(mTable.RowFlags & ImGuiTableRowFlags_Headers))
			mTable.RowBgColorCounter++;
		mTable.IsInsideRow = false;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::DrawBorders() {
		ImGuiWindow* inner_window = mTable.InnerWindow;
		if (!mTable.OuterWindow->ClipRect.Overlaps(mTable.OuterRect))
			return;

		ImDrawList* inner_drawlist = inner_window->DrawList;
		mTable.DrawSplitter.SetCurrentChannel(inner_drawlist, TABLE_DRAW_CHANNEL_BG0);
		inner_drawlist->PushClipRect(mTable.Bg0ClipRectForDrawCmd.Min, mTable.Bg0ClipRectForDrawCmd.Max, false);

		// Draw inner border and resizing feedback
		const float border_size = TABLE_BORDER_SIZE;
		const float draw_y1 = mTable.InnerRect.Min.y;
		const float draw_y2_body = mTable.InnerRect.Max.y;
		const float draw_y2_head = mTable.IsUsingHeaders ? ImMin(mTable.InnerRect.Max.y, (mTable.FreezeRowsCount >= 1 ? mTable.InnerRect.Min.y : mTable.WorkRect.Min.y) + mTable.LastFirstRowHeight) : draw_y1;
		if (mTable.Flags & ImGuiTableFlags_BordersInnerV) {
			for (int order_n = 0; order_n < mTable.ColumnsCount; order_n++) {
				if (!mTable.EnabledMaskByDisplayOrder.test(order_n))
					continue;

				const int column_n = mTable.DisplayOrderToIndex[order_n];
				TableColumn* column = &mTable.Columns[column_n];
				const bool is_hovered = (mTable.HoveredColumnBorder == column_n);
				const bool is_resized = (mTable.ResizedColumn == column_n) && (mTable.InstanceInteracted == mTable.InstanceCurrent);
				const bool is_resizable = (column->Flags & (ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoDirectResize_)) == 0;
				const bool is_frozen_separator = (mTable.FreezeColumnsCount != -1 && mTable.FreezeColumnsCount == order_n + 1);
				if (column->MaxX > mTable.InnerClipRect.Max.x && !is_resized)
					continue;

				// Decide whether right-most column is visible
				if (column->NextEnabledColumn == -1 && !is_resizable)
					if ((mTable.Flags & ImGuiTableFlags_SizingMask_) != ImGuiTableFlags_SizingFixedSame || (mTable.Flags & ImGuiTableFlags_NoHostExtendX))
						continue;
				if (column->MaxX <= column->ClipRect.Min.x) // FIXME-TABLE FIXME-STYLE: Assume BorderSize==1, this is problematic if we want to increase the border size..
					continue;

				// Draw in outer window so right-most column won't be clipped
				// Always draw full height border when being resized/hovered, or on the delimitation of frozen column scrolling.
				ImU32 col;
				float draw_y2;
				if (is_hovered || is_resized || is_frozen_separator) {
					draw_y2 = draw_y2_body;
					col = is_resized ? ImGui::GetColorU32(ImGuiCol_SeparatorActive) : is_hovered ? ImGui::GetColorU32(ImGuiCol_SeparatorHovered)
																								 : mTable.BorderColorStrong;
				} else {
					draw_y2 = (mTable.Flags & (ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoBordersInBodyUntilResize)) ? draw_y2_head : draw_y2_body;
					col = (mTable.Flags & (ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_NoBordersInBodyUntilResize)) ? mTable.BorderColorStrong : mTable.BorderColorLight;
				}

				if (draw_y2 > draw_y1)
					inner_drawlist->AddLine(ImVec2(column->MaxX, draw_y1), ImVec2(column->MaxX, draw_y2), col, border_size);
			}
		}

		// Draw outer border
		// FIXME: could use AddRect or explicit VLine/HLine helper?
		if (mTable.Flags & ImGuiTableFlags_BordersOuter) {
			// Display outer border offset by 1 which is a simple way to display it without adding an extra draw call
			// (Without the offset, in outer_window it would be rendered behind cells, because child windows are above their
			// parent. In inner_window, it won't reach out over scrollbars. Another weird solution would be to display part
			// of it in inner window, and the part that's over scrollbars in the outer window..)
			// Either solution currently won't allow us to use a larger border size: the border would clipped.
			const ImRect outer_border = mTable.OuterRect;
			const ImU32 outer_col = mTable.BorderColorStrong;
			if ((mTable.Flags & ImGuiTableFlags_BordersOuter) == ImGuiTableFlags_BordersOuter) {
				inner_drawlist->AddRect(outer_border.Min, outer_border.Max, outer_col, 0.0f, ~0, border_size);
			} else if (mTable.Flags & ImGuiTableFlags_BordersOuterV) {
				inner_drawlist->AddLine(outer_border.Min, ImVec2(outer_border.Min.x, outer_border.Max.y), outer_col, border_size);
				inner_drawlist->AddLine(ImVec2(outer_border.Max.x, outer_border.Min.y), outer_border.Max, outer_col, border_size);
			} else if (mTable.Flags & ImGuiTableFlags_BordersOuterH) {
				inner_drawlist->AddLine(outer_border.Min, ImVec2(outer_border.Max.x, outer_border.Min.y), outer_col, border_size);
				inner_drawlist->AddLine(ImVec2(outer_border.Min.x, outer_border.Max.y), outer_border.Max, outer_col, border_size);
			}
		}
		if ((mTable.Flags & ImGuiTableFlags_BordersInnerH) && mTable.RowPosY2 < mTable.OuterRect.Max.y) {
			// Draw bottom-most row border
			const float border_y = mTable.RowPosY2;
			if (border_y >= mTable.BgClipRect.Min.y && border_y < mTable.BgClipRect.Max.y)
				inner_drawlist->AddLine(ImVec2(mTable.BorderX1, border_y), ImVec2(mTable.BorderX2, border_y), mTable.BorderColorLight, border_size);
		}

		inner_drawlist->PopClipRect();
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::MergeDrawChannels() {
		ImGuiContext& g = *GImGui;
		ImDrawListSplitter* splitter = &mTable.DrawSplitter;
		const bool has_freeze_v = (mTable.FreezeRowsCount > 0);
		const bool has_freeze_h = (mTable.FreezeColumnsCount > 0);
		IM_ASSERT(splitter->_Current == 0);

		// Track which groups we are going to attempt to merge, and which channels goes into each group.
		struct MergeGroup {
			ImRect ClipRect;
			int ChannelsCount;
			ImBitArray<IMGUI_TABLE_MAX_DRAW_CHANNELS> ChannelsMask;
		};
		int merge_group_mask = 0x00;
		MergeGroup merge_groups[4];
		memset(merge_groups, 0, sizeof(merge_groups));

		// 1. Scan channels and take note of those which can be merged
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			if (!mTable.VisibleMaskByIndex.test(column_n))
				continue;
			TableColumn* column = &mTable.Columns[column_n];

			const int merge_group_sub_count = has_freeze_v ? 2 : 1;
			for (int merge_group_sub_n = 0; merge_group_sub_n < merge_group_sub_count; merge_group_sub_n++) {
				const int channel_no = (merge_group_sub_n == 0) ? column->DrawChannelFrozen : column->DrawChannelUnfrozen;

				// Don't attempt to merge if there are multiple draw calls within the column
				ImDrawChannel* src_channel = &splitter->_Channels[channel_no];
				if (src_channel->_CmdBuffer.Size > 0 && src_channel->_CmdBuffer.back().ElemCount == 0)
					src_channel->_CmdBuffer.pop_back();
				if (src_channel->_CmdBuffer.Size != 1)
					continue;

				// Find out the width of this merge group and check if it will fit in our column
				// (note that we assume that rendering didn't stray on the left direction. we should need a CursorMinPos to detect it)
				if (!(column->Flags & ImGuiTableColumnFlags_NoClip)) {
					float content_max_x;
					if (!has_freeze_v)
						content_max_x = ImMax(column->ContentMaxXUnfrozen, column->ContentMaxXHeadersUsed); // No row freeze
					else if (merge_group_sub_n == 0)
						content_max_x = ImMax(column->ContentMaxXFrozen, column->ContentMaxXHeadersUsed); // Row freeze: use width before freeze
					else
						content_max_x = column->ContentMaxXUnfrozen; // Row freeze: use width after freeze
					if (content_max_x > column->ClipRect.Max.x)
						continue;
				}

				const int merge_group_n = (has_freeze_h && column_n < mTable.FreezeColumnsCount ? 0 : 1) + (has_freeze_v && merge_group_sub_n == 0 ? 0 : 2);
				IM_ASSERT(channel_no < IMGUI_TABLE_MAX_DRAW_CHANNELS);
				MergeGroup* merge_group = &merge_groups[merge_group_n];
				if (merge_group->ChannelsCount == 0)
					merge_group->ClipRect = ImRect(+FLT_MAX, +FLT_MAX, -FLT_MAX, -FLT_MAX);
				merge_group->ChannelsMask.SetBit(channel_no);
				merge_group->ChannelsCount++;
				merge_group->ClipRect.Add(src_channel->_CmdBuffer[0].ClipRect);
				merge_group_mask |= (1 << merge_group_n);
			}

			// Invalidate current draw channel
			// (we don't clear DrawChannelFrozen/DrawChannelUnfrozen solely to facilitate debugging/later inspection of data)
			column->DrawChannelCurrent = (ImGuiTableDrawChannelIdx) -1;
		}

		// 2. Rewrite channel list in our preferred order
		if (merge_group_mask != 0) {
			// We skip channel 0 (Bg0/Bg1) and 1 (Bg2 frozen) from the shuffling since they won't move - see channels allocation in TableSetupDrawChannels().
			const int LEADING_DRAW_CHANNELS = 2;
			g.DrawChannelsTempMergeBuffer.resize(splitter->_Count - LEADING_DRAW_CHANNELS); // Use shared temporary storage so the allocation gets amortized
			ImDrawChannel* dst_tmp = g.DrawChannelsTempMergeBuffer.Data;
			ImBitArray<IMGUI_TABLE_MAX_DRAW_CHANNELS> remaining_mask; // We need 132-bit of storage
			remaining_mask.ClearAllBits();
			remaining_mask.SetBitRange(LEADING_DRAW_CHANNELS, splitter->_Count);
			remaining_mask.ClearBit(mTable.Bg2DrawChannelUnfrozen);
			IM_ASSERT(has_freeze_v == false || mTable.Bg2DrawChannelUnfrozen != TABLE_DRAW_CHANNEL_BG2_FROZEN);
			int remaining_count = splitter->_Count - (has_freeze_v ? LEADING_DRAW_CHANNELS + 1 : LEADING_DRAW_CHANNELS);
			//ImRect host_rect = (mTable.InnerWindow == mTable.OuterWindow) ? mTable.InnerClipRect : mTable.HostClipRect;
			ImRect host_rect = mTable.HostClipRect;
			for (int merge_group_n = 0; merge_group_n < IM_ARRAYSIZE(merge_groups); merge_group_n++) {
				if (int merge_channels_count = merge_groups[merge_group_n].ChannelsCount) {
					MergeGroup* merge_group = &merge_groups[merge_group_n];
					ImRect merge_clip_rect = merge_group->ClipRect;

					// Extend outer-most clip limits to match those of host, so draw calls can be merged even if
					// outer-most columns have some outer padding offsetting them from their parent ClipRect.
					// The principal cases this is dealing with are:
					// - On a same-window table (not scrolling = single group), all fitting columns ClipRect -> will extend and match host ClipRect -> will merge
					// - Columns can use padding and have left-most ClipRect.Min.x and right-most ClipRect.Max.x != from host ClipRect -> will extend and match host ClipRect -> will merge
					// FIXME-TABLE FIXME-WORKRECT: We are wasting a merge opportunity on tables without scrolling if column doesn't fit
					// within host clip rect, solely because of the half-padding difference between window->WorkRect and window->InnerClipRect.
					if ((merge_group_n & 1) == 0 || !has_freeze_h)
						merge_clip_rect.Min.x = ImMin(merge_clip_rect.Min.x, host_rect.Min.x);
					if ((merge_group_n & 2) == 0 || !has_freeze_v)
						merge_clip_rect.Min.y = ImMin(merge_clip_rect.Min.y, host_rect.Min.y);
					if ((merge_group_n & 1) != 0)
						merge_clip_rect.Max.x = ImMax(merge_clip_rect.Max.x, host_rect.Max.x);
					if ((merge_group_n & 2) != 0 && (mTable.Flags & ImGuiTableFlags_NoHostExtendY) == 0)
						merge_clip_rect.Max.y = ImMax(merge_clip_rect.Max.y, host_rect.Max.y);

					remaining_count -= merge_group->ChannelsCount;
					for (int n = 0; n < IM_ARRAYSIZE(remaining_mask.Storage); n++)
						remaining_mask.Storage[n] &= ~merge_group->ChannelsMask.Storage[n];
					for (int n = 0; n < splitter->_Count && merge_channels_count != 0; n++) {
						// Copy + overwrite new clip rect
						if (!merge_group->ChannelsMask.TestBit(n))
							continue;
						merge_group->ChannelsMask.ClearBit(n);
						merge_channels_count--;

						ImDrawChannel* channel = &splitter->_Channels[n];
						IM_ASSERT(channel->_CmdBuffer.Size == 1 && merge_clip_rect.Contains(ImRect(channel->_CmdBuffer[0].ClipRect)));
						channel->_CmdBuffer[0].ClipRect = merge_clip_rect.ToVec4();
						memcpy(dst_tmp++, channel, sizeof(ImDrawChannel));
					}
				}

				// Make sure Bg2DrawChannelUnfrozen appears in the middle of our groups (whereas Bg0/Bg1 and Bg2 frozen are fixed to 0 and 1)
				if (merge_group_n == 1 && has_freeze_v)
					memcpy(dst_tmp++, &splitter->_Channels[mTable.Bg2DrawChannelUnfrozen], sizeof(ImDrawChannel));
			}

			// Append unmergeable channels that we didn't reorder at the end of the list
			for (int n = 0; n < splitter->_Count && remaining_count != 0; n++) {
				if (!remaining_mask.TestBit(n))
					continue;
				ImDrawChannel* channel = &splitter->_Channels[n];
				memcpy(dst_tmp++, channel, sizeof(ImDrawChannel));
				remaining_count--;
			}
			IM_ASSERT(dst_tmp == g.DrawChannelsTempMergeBuffer.Data + g.DrawChannelsTempMergeBuffer.Size);
			memcpy(splitter->_Channels.Data + LEADING_DRAW_CHANNELS, g.DrawChannelsTempMergeBuffer.Data, (splitter->_Count - LEADING_DRAW_CHANNELS) * sizeof(ImDrawChannel));
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	float MainTable<MaxColumnCount>::GetColumnWidthAuto(TableColumn* column) {
		const float content_width_body = ImMax(column->ContentMaxXFrozen, column->ContentMaxXUnfrozen) - column->WorkMinX;
		const float content_width_headers = column->ContentMaxXHeadersIdeal - column->WorkMinX;
		float width_auto = content_width_body;
		if (!(column->Flags & ImGuiTableColumnFlags_NoHeaderWidth))
			width_auto = ImMax(width_auto, content_width_headers);

		// Non-resizable fixed columns preserve their requested width
		if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && column->InitStretchWeightOrWidth > 0.0f)
			if (!(mTable.Flags & ImGuiTableFlags_Resizable) || (column->Flags & ImGuiTableColumnFlags_NoResize))
				width_auto = column->InitStretchWeightOrWidth;

		return ImMax(width_auto, mTable.MinColumnWidth);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SaveSettingsCustom() {
		mTable.IsSettingsDirty = false;
		if (mTable.Flags & ImGuiTableFlags_NoSavedSettings || (getCustomColumnsActive() && mSpecificColumnsActive))
			return;

		// Bind or create settings data
		TableSettings& settings = getTableSettings();

		// Serialize ImGuiTable/ImGuiTableColumn into ImGuiTableSettings/ImGuiTableColumnSettings
		// IM_ASSERT(settings.ID == mTable.ID);
		// IM_ASSERT(settings->ColumnsCount == mTable.ColumnsCount && settings->ColumnsCountMax >= settings->ColumnsCount);
		TableColumn* column = mTable.Columns.Data;

		// clear columns before saving them
		settings.Columns.clear();

		bool save_ref_scale = false;
		settings.SaveFlags = ImGuiTableFlags_None;
		for (int n = 0; n < mTable.ColumnsCount; n++, column++) {
			TableColumnSettings& column_settings = settings.Columns.emplace_back();
			const float width_or_weight = (column->Flags & ImGuiTableColumnFlags_WidthStretch) ? column->StretchWeight : column->WidthRequest;
			column_settings.WidthOrWeight = width_or_weight;
			column_settings.UserID = column->UserID;
			// column_settings.Index = (TableColumnIdx)n;
			column_settings.DisplayOrder = column->DisplayOrder;
			column_settings.SortOrder = column->SortOrder;
			column_settings.SortDirection = column->SortDirection;
			column_settings.IsEnabled = column->IsEnabled;
			column_settings.IsStretch = (column->Flags & ImGuiTableColumnFlags_WidthStretch) ? 1 : 0;
			if ((column->Flags & ImGuiTableColumnFlags_WidthStretch) == 0)
				save_ref_scale = true;

			// We skip saving some data in the .ini file when they are unnecessary to restore our state.
			// Note that fixed width where initial width was derived from auto-fit will always be saved as InitStretchWeightOrWidth will be 0.0f.
			// FIXME-TABLE: We don't have logic to easily compare SortOrder to DefaultSortOrder yet so it's always saved when present.
			if (width_or_weight != column->InitStretchWeightOrWidth)
				settings.SaveFlags |= ImGuiTableFlags_Resizable;
			if (column->DisplayOrder != n)
				settings.SaveFlags |= ImGuiTableFlags_Reorderable;
			if (column->SortOrder != -1)
				settings.SaveFlags |= ImGuiTableFlags_Sortable;
			if (column->IsEnabled != ((column->Flags & ImGuiTableColumnFlags_DefaultHide) == 0))
				settings.SaveFlags |= ImGuiTableFlags_Hideable;
		}
		settings.SaveFlags &= mTable.Flags;
		settings.RefScale = save_ref_scale ? mTable.RefScale : 0.0f;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::MigrateIniSettings() {
		bool backup = mSpecificColumnsUpdate;
		mSpecificColumnsActive = false;

		TableSettings& tableSettings = getTableSettings();

		// move all settings to the new object, without changing the Table object
		ImGuiContext& g = *GImGui;

		// Bind settings
		ImGuiTableSettings* settings;
		if (mTable.SettingsOffset == -1) {
			settings = ImGui::TableSettingsFindByID(mTable.ID);
			if (settings == NULL)
				return;
			mTable.SettingsOffset = g.SettingsTables.offset_from_ptr(settings);
		} else {
			settings = GetBoundSettings();
		}

		tableSettings.SaveFlags = settings->SaveFlags;
		tableSettings.RefScale = settings->RefScale;

		tableSettings.Columns.resize(settings->ColumnsCount);

		// Serialize ImGuiTableSettings/ImGuiTableColumnSettings into ImGuiTable/ImGuiTableColumn
		ImGuiTableColumnSettings* column_settings = settings->GetColumnSettings();
		for (int data_n = 0; data_n < settings->ColumnsCount; data_n++, column_settings++) {
			int column_n = column_settings->Index;
			if (column_n < 0 || column_n >= mTable.ColumnsCount)
				continue;

			auto& column = tableSettings.Columns[column_n];

			column.DisplayOrder = column_settings->DisplayOrder;
			column.SortOrder = column_settings->SortOrder;
			column.IsEnabled = column_settings->IsEnabled;
			column.IsStretch = column_settings->IsStretch;
			column.SortDirection = column_settings->SortDirection;
			column.UserID = column_settings->UserID;
			column.WidthOrWeight = column_settings->WidthOrWeight;
		}

		mSpecificColumnsActive = backup;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetupColumnFlags(TableColumn* column, ImGuiTableColumnFlags flags_in) {
		ImGuiTableColumnFlags flags = flags_in;

		// Sizing Policy
		if ((flags & ImGuiTableColumnFlags_WidthMask_) == 0) {
			const ImGuiTableFlags table_sizing_policy = (mTable.Flags & ImGuiTableFlags_SizingMask_);
			if (table_sizing_policy == ImGuiTableFlags_SizingFixedFit || table_sizing_policy == ImGuiTableFlags_SizingFixedSame)
				flags |= ImGuiTableColumnFlags_WidthFixed;
			else
				flags |= ImGuiTableColumnFlags_WidthStretch;
		} else {
			IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_WidthMask_)); // Check that only 1 of each set is used.
		}

		// Resize
		if ((mTable.Flags & ImGuiTableFlags_Resizable) == 0)
			flags |= ImGuiTableColumnFlags_NoResize;

		// Sorting
		if ((flags & ImGuiTableColumnFlags_NoSortAscending) && (flags & ImGuiTableColumnFlags_NoSortDescending))
			flags |= ImGuiTableColumnFlags_NoSort;

		// Indentation
		if ((flags & ImGuiTableColumnFlags_IndentMask_) == 0)
			flags |= (mTable.Columns.index_from_ptr(column) == 0) ? ImGuiTableColumnFlags_IndentEnable : ImGuiTableColumnFlags_IndentDisable;

		// Alignment
		//if ((flags & ImGuiTableColumnFlags_AlignMask_) == 0)
		//    flags |= ImGuiTableColumnFlags_AlignCenter;
		//IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiTableColumnFlags_AlignMask_)); // Check that only 1 of each set is used.

		// Preserve status flags
		column->Flags = flags | (column->Flags & ImGuiTableColumnFlags_StatusMask_);

		// Build an ordered list of available sort directions
		column->SortDirectionsAvailCount = column->SortDirectionsAvailMask = column->SortDirectionsAvailList = 0;
		if (mTable.Flags & ImGuiTableFlags_Sortable) {
			int count = 0, mask = 0, list = 0;
			if ((flags & ImGuiTableColumnFlags_PreferSortAscending) != 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) {
				mask |= 1 << ImGuiSortDirection_Ascending;
				list |= ImGuiSortDirection_Ascending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortDescending) != 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) {
				mask |= 1 << ImGuiSortDirection_Descending;
				list |= ImGuiSortDirection_Descending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortAscending) == 0 && (flags & ImGuiTableColumnFlags_NoSortAscending) == 0) {
				mask |= 1 << ImGuiSortDirection_Ascending;
				list |= ImGuiSortDirection_Ascending << (count << 1);
				count++;
			}
			if ((flags & ImGuiTableColumnFlags_PreferSortDescending) == 0 && (flags & ImGuiTableColumnFlags_NoSortDescending) == 0) {
				mask |= 1 << ImGuiSortDirection_Descending;
				list |= ImGuiSortDirection_Descending << (count << 1);
				count++;
			}
			if ((mTable.Flags & ImGuiTableFlags_SortTristate) || count == 0) {
				mask |= 1 << ImGuiSortDirection_None;
				count++;
			}
			column->SortDirectionsAvailList = (ImU8) list;
			column->SortDirectionsAvailMask = (ImU8) mask;
			column->SortDirectionsAvailCount = (ImU8) count;
			FixColumnSortDirection(column);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	float MainTable<MaxColumnCount>::GetMaxColumnWidth(int column_n) {
		const TableColumn* column = &mTable.Columns[column_n];
		float max_width = FLT_MAX;
		const float min_column_distance = mTable.MinColumnWidth + mTable.CellPaddingX * 2.0f + mTable.CellSpacingX1 + mTable.CellSpacingX2;
		if (mTable.Flags & ImGuiTableFlags_ScrollX) {
			// Frozen columns can't reach beyond visible width else scrolling will naturally break.
			if (column->DisplayOrder < mTable.FreezeColumnsRequest) {
				max_width = (mTable.InnerClipRect.Max.x - (mTable.FreezeColumnsRequest - column->DisplayOrder) * min_column_distance) - column->MinX;
				max_width = max_width - mTable.OuterPaddingX - mTable.CellPaddingX - mTable.CellSpacingX2;
			}
		} else if ((mTable.Flags & ImGuiTableFlags_NoKeepColumnsVisible) == 0) {
			// If horizontal scrolling if disabled, we apply a final lossless shrinking of columns in order to make
			// sure they are all visible. Because of this we also know that all of the columns will always fit in
			// mTable.WorkRect and therefore in mTable.InnerRect (because ScrollX is off)
			// FIXME-TABLE: This is solved incorrectly but also quite a difficult problem to fix as we also want ClipRect width to match.
			// See "table_width_distrib" and "table_width_keep_visible" tests
			max_width = mTable.WorkRect.Max.x - (mTable.ColumnsEnabledCount - column->IndexWithinEnabledSet - 1) * min_column_distance - column->MinX;
			//max_width -= mTable.CellSpacingX1;
			max_width -= mTable.CellSpacingX2;
			max_width -= mTable.CellPaddingX * 2.0f;
			max_width -= mTable.OuterPaddingX;
		}
		return max_width;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetupDrawChannels() {
		const int freeze_row_multiplier = (mTable.FreezeRowsCount > 0) ? 2 : 1;
		const int channels_for_row = (mTable.Flags & ImGuiTableFlags_NoClip) ? 1 : mTable.ColumnsEnabledCount;
		const int channels_for_bg = 1 + 1 * freeze_row_multiplier;
		const int channels_for_dummy = (mTable.ColumnsEnabledCount < mTable.ColumnsCount || mTable.VisibleMaskByIndex != mTable.EnabledMaskByIndex) ? +1 : 0;
		const int channels_total = channels_for_bg + (channels_for_row * freeze_row_multiplier) + channels_for_dummy;
		mTable.DrawSplitter.Split(mTable.InnerWindow->DrawList, channels_total);
		mTable.DummyDrawChannel = (ImGuiTableDrawChannelIdx) ((channels_for_dummy > 0) ? channels_total - 1 : -1);
		mTable.Bg2DrawChannelCurrent = TABLE_DRAW_CHANNEL_BG2_FROZEN;
		mTable.Bg2DrawChannelUnfrozen = (ImGuiTableDrawChannelIdx) ((mTable.FreezeRowsCount > 0) ? 2 + channels_for_row : TABLE_DRAW_CHANNEL_BG2_FROZEN);

		int draw_channel_current = 2;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			TableColumn* column = &mTable.Columns[column_n];
			if (column->IsVisibleX && column->IsVisibleY) {
				column->DrawChannelFrozen = (ImGuiTableDrawChannelIdx) (draw_channel_current);
				column->DrawChannelUnfrozen = (ImGuiTableDrawChannelIdx) (draw_channel_current + (mTable.FreezeRowsCount > 0 ? channels_for_row + 1 : 0));
				if (!(mTable.Flags & ImGuiTableFlags_NoClip))
					draw_channel_current++;
			} else {
				column->DrawChannelFrozen = column->DrawChannelUnfrozen = mTable.DummyDrawChannel;
			}
			column->DrawChannelCurrent = column->DrawChannelFrozen;
		}

		// Initial draw cmd starts with a BgClipRect that matches the one of its host, to facilitate merge draw commands by default.
		// All our cell highlight are manually clipped with BgClipRect. When unfreezing it will be made smaller to fit scrolling rect.
		// (This technically isn't part of setting up draw channels, but is reasonably related to be done here)
		mTable.BgClipRect = mTable.InnerClipRect;
		mTable.Bg0ClipRectForDrawCmd = mTable.OuterWindow->ClipRect;
		mTable.Bg2ClipRectForDrawCmd = mTable.HostClipRect;
		IM_ASSERT(mTable.BgClipRect.Min.y <= mTable.BgClipRect.Max.y);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::UpdateBorders() {
		ImGuiContext& g = *GImGui;
		IM_ASSERT(mTable.Flags & ImGuiTableFlags_Resizable);

		// At this point OuterRect height may be zero or under actual final height, so we rely on temporal coherency and
		// use the final height from last frame. Because this is only affecting _interaction_ with columns, it is not
		// really problematic (whereas the actual visual will be displayed in EndTable() and using the current frame height).
		// Actual columns highlight/render will be performed in EndTable() and not be affected.
		const float hit_half_width = TABLE_RESIZE_SEPARATOR_HALF_THICKNESS;
		const float hit_y1 = mTable.OuterRect.Min.y;
		const float hit_y2_body = ImMax(mTable.OuterRect.Max.y, hit_y1 + mTable.LastOuterHeight);
		const float hit_y2_head = hit_y1 + mTable.LastFirstRowHeight;

		for (int order_n = 0; order_n < mTable.ColumnsCount; order_n++) {
			if (!mTable.EnabledMaskByDisplayOrder.test(order_n))
				continue;

			const int column_n = mTable.DisplayOrderToIndex[order_n];
			TableColumn* column = &mTable.Columns[column_n];
			if (column->Flags & (ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoDirectResize_))
				continue;

			// ImGuiTableFlags_NoBordersInBodyUntilResize will be honored in TableDrawBorders()
			const float border_y2_hit = (mTable.Flags & ImGuiTableFlags_NoBordersInBody) ? hit_y2_head : hit_y2_body;
			if ((mTable.Flags & ImGuiTableFlags_NoBordersInBody) && mTable.IsUsingHeaders == false)
				continue;

			if (mTable.FreezeColumnsCount > 0)
				if (column->MaxX < mTable.Columns[mTable.DisplayOrderToIndex[mTable.FreezeColumnsCount - 1]].MaxX)
					continue;

			ImGuiID column_id = GetColumnResizeID(column_n, mTable.InstanceCurrent);
			ImRect hit_rect(column->MaxX - hit_half_width, hit_y1, column->MaxX + hit_half_width, border_y2_hit);
			//GetForegroundDrawList()->AddRect(hit_rect.Min, hit_rect.Max, IM_COL32(255, 0, 0, 100));
			ImGui::KeepAliveID(column_id);

			bool hovered = false, held = false;
			bool pressed = ImGui::ButtonBehavior(hit_rect, column_id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnDoubleClick);
			if (pressed && ImGui::IsMouseDoubleClicked(0)) {
				SetColumnWidthAutoSingle(column_n);
				ImGui::ClearActiveID();
				held = hovered = false;
			}
			if (held) {
				if (mTable.LastResizedColumn == -1)
					mTable.ResizeLockMinContentsX2 = mTable.RightMostEnabledColumn != -1 ? mTable.Columns[mTable.RightMostEnabledColumn].MaxX : -FLT_MAX;
				mTable.ResizedColumn = (TableColumnIdx) column_n;
				mTable.InstanceInteracted = mTable.InstanceCurrent;
			}
			if ((hovered && g.HoveredIdTimer > TABLE_RESIZE_SEPARATOR_FEEDBACK_TIMER) || held) {
				mTable.HoveredColumnBorder = (TableColumnIdx) column_n;
				ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SortSpecsSanitize() {
		IM_ASSERT(mTable.Flags & ImGuiTableFlags_Sortable);

		// Clear SortOrder from hidden column and verify that there's no gap or duplicate.
		int sort_order_count = 0;
		ColumnBitMask sort_order_mask;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			TableColumn* column = &mTable.Columns[column_n];
			if (column->SortOrder != -1 && !column->IsEnabled)
				column->SortOrder = -1;
			if (column->SortOrder == -1)
				continue;
			sort_order_count++;
			sort_order_mask.set(column->SortOrder);
		}

		ColumnBitMask expected;
		for (int i = 0; i < sort_order_count; ++i) {
			expected.set(i);
		}

		const bool need_fix_linearize = expected != sort_order_mask;
		const bool need_fix_single_sort_order = (sort_order_count > 1) && !(mTable.Flags & ImGuiTableFlags_SortMulti);
		if (need_fix_linearize || need_fix_single_sort_order) {
			ColumnBitMask fixed_mask;
			for (int sort_n = 0; sort_n < sort_order_count; sort_n++) {
				// Fix: Rewrite sort order fields if needed so they have no gap or duplicate.
				// (e.g. SortOrder 0 disappeared, SortOrder 1..2 exists --> rewrite then as SortOrder 0..1)
				int column_with_smallest_sort_order = -1;
				for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
					if (!fixed_mask.test(column_n) && mTable.Columns[column_n].SortOrder != -1)
						if (column_with_smallest_sort_order == -1 || mTable.Columns[column_n].SortOrder < mTable.Columns[column_with_smallest_sort_order].SortOrder)
							column_with_smallest_sort_order = column_n;
				IM_ASSERT(column_with_smallest_sort_order != -1);
				fixed_mask.set(column_with_smallest_sort_order);
				mTable.Columns[column_with_smallest_sort_order].SortOrder = (TableColumnIdx) sort_n;

				// Fix: Make sure only one column has a SortOrder if ImGuiTableFlags_MultiSortable is not set.
				if (need_fix_single_sort_order) {
					sort_order_count = 1;
					for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
						if (column_n != column_with_smallest_sort_order)
							mTable.Columns[column_n].SortOrder = -1;
					break;
				}
			}
		}

		// Fallback default sort order (if no column had the ImGuiTableColumnFlags_DefaultSort flag)
		if (sort_order_count == 0 && !(mTable.Flags & ImGuiTableFlags_SortTristate))
			for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
				TableColumn* column = &mTable.Columns[column_n];
				if (column->IsEnabled && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
					sort_order_count = 1;
					column->SortOrder = 0;
					column->SortDirection = (ImU8) GetColumnAvailSortDirection(column, 0);
					break;
				}
			}

		mTable.SortSpecsCount = (TableColumnIdx) sort_order_count;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::FixColumnSortDirection(TableColumn* column) {
		if (column->SortOrder == -1 || (column->SortDirectionsAvailMask & (1 << column->SortDirection)) != 0)
			return;
		column->SortDirection = (ImU8) GetColumnAvailSortDirection(column, 0);
		mTable.IsSortSpecsDirty = true;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::EndCell() {
		TableColumn* column = &mTable.Columns[mTable.CurrentColumn];
		ImGuiWindow* window = mTable.InnerWindow;

		// Report maximum position so we can infer content size per column.
		float* p_max_pos_x;
		if (mTable.RowFlags & ImGuiTableRowFlags_Headers)
			p_max_pos_x = &column->ContentMaxXHeadersUsed; // Useful in case user submit contents in header row that is not a TableHeader() call
		else
			p_max_pos_x = mTable.IsUnfrozenRows ? &column->ContentMaxXUnfrozen : &column->ContentMaxXFrozen;
		*p_max_pos_x = ImMax(*p_max_pos_x, window->DC.CursorMaxPos.x);
		mTable.RowPosY2 = ImMax(mTable.RowPosY2, window->DC.CursorMaxPos.y + mTable.CellPaddingY);
		column->ItemWidth = window->DC.ItemWidth;

		// Propagate text baseline for the entire row
		// FIXME-TABLE: Here we propagate text baseline from the last line of the cell.. instead of the first one.
		mTable.RowTextBaseline = ImMax(mTable.RowTextBaseline, window->DC.PrevLineTextBaseOffset);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::BeginCell(int column_n) {
		TableColumn* column = &mTable.Columns[column_n];
		ImGuiWindow* window = mTable.InnerWindow;
		mTable.CurrentColumn = column_n;

		// Start position is roughly ~~ CellRect.Min + CellPadding + Indent
		float start_x = column->WorkMinX;
		if (column->Flags & ImGuiTableColumnFlags_IndentEnable)
			start_x += mTable.RowIndentOffsetX; // ~~ += window.DC.Indent.x - mTable.HostIndentX, except we locked it for the row.

		window->DC.CursorPos.x = start_x;
		window->DC.CursorPos.y = mTable.RowPosY1 + mTable.CellPaddingY;
		window->DC.CursorMaxPos.x = window->DC.CursorPos.x;
		window->DC.ColumnsOffset.x = start_x - window->Pos.x - window->DC.Indent.x; // FIXME-WORKRECT
		window->DC.CurrLineTextBaseOffset = mTable.RowTextBaseline;
		window->DC.NavLayerCurrent = (ImGuiNavLayer) column->NavLayerCurrent;

		window->WorkRect.Min.y = window->DC.CursorPos.y;
		window->WorkRect.Min.x = column->WorkMinX;
		window->WorkRect.Max.x = column->WorkMaxX;
		window->DC.ItemWidth = column->ItemWidth;

		// To allow ImGuiListClipper to function we propagate our row height
		if (!column->IsEnabled)
			window->DC.CursorPos.y = ImMax(window->DC.CursorPos.y, mTable.RowPosY2);

		window->SkipItems = column->IsSkipItems;
		if (column->IsSkipItems) {
			window->DC.LastItemId = 0;
			window->DC.LastItemStatusFlags = 0;
		}

		if (mTable.Flags & ImGuiTableFlags_NoClip) {
			// FIXME: if we end up drawing all borders/bg in EndTable, could remove this and just assert that channel hasn't changed.
			mTable.DrawSplitter.SetCurrentChannel(window->DrawList, TABLE_DRAW_CHANNEL_NOCLIP);
			//IM_ASSERT(mTable.DrawSplitter._Current == TABLE_DRAW_CHANNEL_NOCLIP);
		} else {
			// FIXME-TABLE: Could avoid this if draw channel is dummy channel?
			ImGui::SetWindowClipRectBeforeSetChannel(window, column->ClipRect);
			mTable.DrawSplitter.SetCurrentChannel(window->DrawList, column->DrawChannelCurrent);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::BeginRow() {
		ImGuiWindow* window = mTable.InnerWindow;
		IM_ASSERT(!mTable.IsInsideRow);

		// New row
		mTable.CurrentRow++;
		mTable.CurrentColumn = -1;
		mTable.RowBgColor[0] = mTable.RowBgColor[1] = IM_COL32_DISABLE;
		mTable.RowCellDataCurrent = -1;
		mTable.IsInsideRow = true;

		// Begin frozen rows
		float next_y1 = mTable.RowPosY2;
		if (mTable.CurrentRow == 0 && mTable.FreezeRowsCount > 0)
			next_y1 = window->DC.CursorPos.y = mTable.OuterRect.Min.y;

		mTable.RowPosY1 = mTable.RowPosY2 = next_y1;
		mTable.RowTextBaseline = 0.0f;
		mTable.RowIndentOffsetX = window->DC.Indent.x - mTable.HostIndentX; // Lock indent
		window->DC.PrevLineTextBaseOffset = 0.0f;
		window->DC.CursorMaxPos.y = next_y1;

		// Making the header BG color non-transparent will allow us to overlay it multiple times when handling smooth dragging.
		if (mTable.RowFlags & ImGuiTableRowFlags_Headers) {
			SetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
			if (mTable.CurrentRow == 0)
				mTable.IsUsingHeaders = true;
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n) {
		IM_ASSERT(target != ImGuiTableBgTarget_None);

		if (color == IM_COL32_DISABLE)
			color = 0;

		// We cannot draw neither the cell or row background immediately as we don't know the row height at this point in time.
		switch (target) {
			case ImGuiTableBgTarget_CellBg: {
				if (mTable.RowPosY1 > mTable.InnerClipRect.Max.y) // Discard
					return;
				if (column_n == -1)
					column_n = mTable.CurrentColumn;
				if (!mTable.VisibleMaskByIndex.test(column_n))
					return;
				if (mTable.RowCellDataCurrent < 0 || mTable.RowCellData[mTable.RowCellDataCurrent].Column != column_n)
					mTable.RowCellDataCurrent++;
				ImGuiTableCellData* cell_data = &mTable.RowCellData[mTable.RowCellDataCurrent];
				cell_data->BgColor = color;
				cell_data->Column = (TableColumnIdx) column_n;
				break;
			}
			case ImGuiTableBgTarget_RowBg0:
			case ImGuiTableBgTarget_RowBg1: {
				if (mTable.RowPosY1 > mTable.InnerClipRect.Max.y) // Discard
					return;
				IM_ASSERT(column_n == -1);
				int bg_idx = (target == ImGuiTableBgTarget_RowBg1) ? 1 : 0;
				mTable.RowBgColor[bg_idx] = color;
				break;
			}
			default:
				IM_ASSERT(0);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetColumnSortDirection(int column_n, ImGuiSortDirection sort_direction, bool append_to_sort_specs) {
		if (!(mTable.Flags & ImGuiTableFlags_SortMulti))
			append_to_sort_specs = false;
		if (!(mTable.Flags & ImGuiTableFlags_SortTristate))
			IM_ASSERT(sort_direction != ImGuiSortDirection_None);

		TableColumnIdx sort_order_max = 0;
		if (append_to_sort_specs)
			for (int other_column_n = 0; other_column_n < mTable.ColumnsCount; other_column_n++)
				sort_order_max = ImMax(sort_order_max, mTable.Columns[other_column_n].SortOrder);

		TableColumn* column = &mTable.Columns[column_n];
		column->SortDirection = (ImU8) sort_direction;
		if (column->SortDirection == ImGuiSortDirection_None)
			column->SortOrder = -1;
		else if (column->SortOrder == -1 || !append_to_sort_specs)
			column->SortOrder = append_to_sort_specs ? sort_order_max + 1 : 0;

		for (int other_column_n = 0; other_column_n < mTable.ColumnsCount; other_column_n++) {
			TableColumn* other_column = &mTable.Columns[other_column_n];
			if (other_column != column && !append_to_sort_specs)
				other_column->SortOrder = -1;
			FixColumnSortDirection(other_column);
		}
		mTable.IsSettingsDirty = true;
		mTable.IsSortSpecsDirty = true;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	ImGuiSortDirection MainTable<MaxColumnCount>::GetColumnNextSortDirection(TableColumn* column) {
		IM_ASSERT(column->SortDirectionsAvailCount > 0);
		if (column->SortOrder == -1)
			return GetColumnAvailSortDirection(column, 0);
		for (int n = 0; n < 3; n++)
			if (column->SortDirection == GetColumnAvailSortDirection(column, n))
				return GetColumnAvailSortDirection(column, (n + 1) % column->SortDirectionsAvailCount);
		IM_ASSERT(0);
		return ImGuiSortDirection_None;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetColumnWidth(int column_n, float width) {
		IM_ASSERT(mTable.IsLayoutLocked == false);
		IM_ASSERT(column_n >= 0 && column_n < mTable.ColumnsCount);
		TableColumn* column_0 = &mTable.Columns[column_n];
		float column_0_width = width;

		// Apply constraints early
		// Compare both requested and actual given width to avoid overwriting requested width when column is stuck (minimum size, bounded)
		IM_ASSERT(mTable.MinColumnWidth > 0.0f);
		const float min_width = mTable.MinColumnWidth;
		const float max_width = ImMax(min_width, GetMaxColumnWidth(column_n));
		column_0_width = ImClamp(column_0_width, min_width, max_width);
		if (column_0->WidthGiven == column_0_width || column_0->WidthRequest == column_0_width)
			return;

		//IMGUI_DEBUG_LOG("TableSetColumnWidth(%d, %.1f->%.1f)\n", column_0_idx, column_0->WidthGiven, column_0_width);
		TableColumn* column_1 = (column_0->NextEnabledColumn != -1) ? &mTable.Columns[column_0->NextEnabledColumn] : NULL;

		// In this surprisingly not simple because of how we support mixing Fixed and multiple Stretch columns.
		// - All fixed: easy.
		// - All stretch: easy.
		// - One or more fixed + one stretch: easy.
		// - One or more fixed + more than one stretch: tricky.
		// Qt when manual resize is enabled only support a single _trailing_ stretch column.

		// When forwarding resize from Wn| to Fn+1| we need to be considerate of the _NoResize flag on Fn+1.
		// FIXME-TABLE: Find a way to rewrite all of this so interactions feel more consistent for the user.
		// Scenarios:
		// - F1 F2 F3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. Subsequent columns will be offset.
		// - F1 F2 F3  resize from F3|          --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered.
		// - F1 F2 W3  resize from F1| or F2|   --> ok: alter ->WidthRequested of Fixed column. If active, ScrollX extent can be altered, but it doesn't make much sense as the Stretch column will always be minimal size.
		// - F1 F2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
		// - W1 W2 W3  resize from W1| or W2|   --> ok
		// - W1 W2 W3  resize from W3|          --> ok: no-op (disabled by Resize Rule 1)
		// - W1 F2 F3  resize from F3|          --> ok: no-op (disabled by Resize Rule 1)
		// - W1 F2     resize from F2|          --> ok: no-op (disabled by Resize Rule 1)
		// - W1 W2 F3  resize from W1| or W2|   --> ok
		// - W1 F2 W3  resize from W1| or F2|   --> ok
		// - F1 W2 F3  resize from W2|          --> ok
		// - F1 W3 F2  resize from W3|          --> ok
		// - W1 F2 F3  resize from W1|          --> ok: equivalent to resizing |F2. F3 will not move.
		// - W1 F2 F3  resize from F2|          --> ok
		// All resizes from a Wx columns are locking other columns.

		// Possible improvements:
		// - W1 W2 W3  resize W1|               --> to not be stuck, both W2 and W3 would stretch down. Seems possible to fix. Would be most beneficial to simplify resize of all-weighted columns.
		// - W3 F1 F2  resize W3|               --> to not be stuck past F1|, both F1 and F2 would need to stretch down, which would be lossy or ambiguous. Seems hard to fix.

		// [Resize Rule 1] Can't resize from right of right-most visible column if there is any Stretch column. Implemented in TableUpdateLayout().

		// If we have all Fixed columns OR resizing a Fixed column that doesn't come after a Stretch one, we can do an offsetting resize.
		// This is the preferred resize path
		if (column_0->Flags & ImGuiTableColumnFlags_WidthFixed)
			if (!column_1 || mTable.LeftMostStretchedColumn == -1 || mTable.Columns[mTable.LeftMostStretchedColumn].DisplayOrder >= column_0->DisplayOrder) {
				column_0->WidthRequest = column_0_width;
				mTable.IsSettingsDirty = true;
				return;
			}

		// We can also use previous column if there's no next one (this is used when doing an auto-fit on the right-most stretch column)
		if (column_1 == NULL)
			column_1 = (column_0->PrevEnabledColumn != -1) ? &mTable.Columns[column_0->PrevEnabledColumn] : NULL;
		if (column_1 == NULL)
			return;

		// Resizing from right-side of a Stretch column before a Fixed column forward sizing to left-side of fixed column.
		// (old_a + old_b == new_a + new_b) --> (new_a == old_a + old_b - new_b)
		float column_1_width = ImMax(column_1->WidthRequest - (column_0_width - column_0->WidthRequest), min_width);
		column_0_width = column_0->WidthRequest + column_1->WidthRequest - column_1_width;
		IM_ASSERT(column_0_width > 0.0f && column_1_width > 0.0f);
		column_0->WidthRequest = column_0_width;
		column_1->WidthRequest = column_1_width;
		if ((column_0->Flags | column_1->Flags) & ImGuiTableColumnFlags_WidthStretch)
			UpdateColumnsWeightFromWidth();
		mTable.IsSettingsDirty = true;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::UpdateColumnsWeightFromWidth() {
		IM_ASSERT(mTable.LeftMostStretchedColumn != -1 && mTable.RightMostStretchedColumn != -1);

		// Measure existing quantity
		float visible_weight = 0.0f;
		float visible_width = 0.0f;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			TableColumn* column = &mTable.Columns[column_n];
			if (!column->IsEnabled || !(column->Flags & ImGuiTableColumnFlags_WidthStretch))
				continue;
			IM_ASSERT(column->StretchWeight > 0.0f);
			visible_weight += column->StretchWeight;
			visible_width += column->WidthRequest;
		}
		IM_ASSERT(visible_weight > 0.0f && visible_width > 0.0f);

		// Apply new weights
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++) {
			TableColumn* column = &mTable.Columns[column_n];
			if (!column->IsEnabled || !(column->Flags & ImGuiTableColumnFlags_WidthStretch))
				continue;
			column->StretchWeight = (column->WidthRequest / visible_width) * visible_weight;
			IM_ASSERT(column->StretchWeight > 0.0f);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	bool MainTable<MaxColumnCount>::IsCurrentColumnHovered() {
		const ImRect& cellBgRect = GetCellBgRect(mTable.CurrentColumn);

		// [[ DEBUG ]]
		// ImGui::GetCurrentWindow()->DrawList->AddRect(cellBgRect.Min, cellBgRect.Max), 0xff0000ff);

		return ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(cellBgRect.Min, cellBgRect.Max);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	bool MainTable<MaxColumnCount>::IsCurrentRowHovered() {
		return mTable.CurrentRow == mTable.CurrentHoveredRow;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	int MainTable<MaxColumnCount>::GetColumnIndex() {
		return mTable.CurrentColumn;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	bool MainTable<MaxColumnCount>::Begin(const char* str_id, int columns_count, ImGuiTableFlags flags, const ImVec2& outer_size, float inner_width, ImGuiWindowFlags child_window_flags) {
		ImGuiID id = ImGui::GetID(str_id);

		// add ScrollY, we want to have scrollbars
		if (mFlags & MainTableFlags_SubWindow)
			flags |= ImGuiTableFlags_ScrollY;

		ImGuiContext& g = *GImGui;
		ImGuiWindow* outer_window = ImGui::GetCurrentWindow();
		if (outer_window->SkipItems) // Consistent with other tables + beneficial side effect that assert on miscalling EndTable() will be more visible.
			return false;

		// Sanity checks
		IM_ASSERT(columns_count > 0 && columns_count <= MaxColumnCount && "Only 1..MaxColumnCount columns allowed!");
		if (flags & ImGuiTableFlags_ScrollX)
			IM_ASSERT(inner_width >= 0.0f);

		// If an outer size is specified ahead we will be able to early out when not visible. Exact clipping rules may evolve.
		const bool use_child_window = (flags & (ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) != 0;
		const ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImVec2 actual_outer_size = ImGui::CalcItemSize(outer_size, ImMax(avail_size.x, 1.0f), use_child_window ? ImMax(avail_size.y, 1.0f) : 0.0f);
		ImRect outer_rect(outer_window->DC.CursorPos, outer_window->DC.CursorPos + actual_outer_size);

		if (use_child_window && ImGui::IsClippedEx(outer_rect, 0, false)) {
			ImGui::ItemSize(outer_rect);
			// This causes issues with the killproof.me plugin.
			// Removing this return will fix those, if you find issues with the removal of this return tell me.
			// ~knox
			// return false;
		}

		// Acquire storage for the table
		const int instance_no = (mTable.LastFrameActive != g.FrameCount) ? 0 : mTable.InstanceCurrent + 1;
		const ImGuiID instance_id = id + instance_no;
		const ImGuiTableFlags table_last_flags = mTable.Flags;
		if (instance_no > 0)
			IM_ASSERT(mTable.ColumnsCount == columns_count && "BeginTable(): Cannot change columns count mid-frame while preserving same ID");

		// Fix flags
		mTable.IsDefaultSizingPolicy = (flags & ImGuiTableFlags_SizingMask_) == 0;
		flags = TableFixFlags(flags, outer_window);

		// Initialize
		mTable.ID = id;
		mTable.Flags = flags;
		mTable.InstanceCurrent = (ImS16) instance_no;
		mTable.LastFrameActive = g.FrameCount;
		mTable.OuterWindow = mTable.InnerWindow = outer_window;
		mTable.ColumnsCount = columns_count;
		mTable.IsLayoutLocked = false;
		mTable.InnerWidth = inner_width;
		mTable.UserOuterSize = outer_size;

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
			child_flags |= getShowScrollbar() ? 0 : ImGuiWindowFlags_NoScrollbar;
			child_flags |= (flags & ImGuiTableFlags_ScrollX) ? ImGuiWindowFlags_HorizontalScrollbar : ImGuiWindowFlags_None;
			ImGui::BeginChildEx(str_id, instance_id, outer_rect.GetSize(), false, child_flags);
			mTable.InnerWindow = g.CurrentWindow;
			mTable.WorkRect = mTable.InnerWindow->WorkRect;
			mTable.OuterRect = mTable.InnerWindow->Rect();
			mTable.InnerRect = mTable.InnerWindow->InnerRect;
			IM_ASSERT(mTable.InnerWindow->WindowPadding.x == 0.0f && mTable.InnerWindow->WindowPadding.y == 0.0f && mTable.InnerWindow->WindowBorderSize == 0.0f);
		} else {
			// For non-scrolling tables, WorkRect == OuterRect == InnerRect.
			// But at this point we do NOT have a correct value for .Max.y (unless a height has been explicitly passed in). It will only be updated in EndTable().
			mTable.WorkRect = mTable.OuterRect = mTable.InnerRect = outer_rect;
		}

		// Push a standardized ID for both child-using and not-child-using tables
		ImGui::PushOverrideID(instance_id);

		// Backup a copy of host window members we will modify
		ImGuiWindow* inner_window = mTable.InnerWindow;
		mTable.HostIndentX = inner_window->DC.Indent.x;
		mTable.HostClipRect = inner_window->ClipRect;
		mTable.HostSkipItems = inner_window->SkipItems;
		mTable.HostBackupWorkRect = inner_window->WorkRect;
		mTable.HostBackupParentWorkRect = inner_window->ParentWorkRect;
		mTable.HostBackupColumnsOffset = outer_window->DC.ColumnsOffset;
		mTable.HostBackupPrevLineSize = inner_window->DC.PrevLineSize;
		mTable.HostBackupCurrLineSize = inner_window->DC.CurrLineSize;
		mTable.HostBackupCursorMaxPos = inner_window->DC.CursorMaxPos;
		mTable.HostBackupItemWidth = outer_window->DC.ItemWidth;
		mTable.HostBackupItemWidthStackSize = outer_window->DC.ItemWidthStack.Size;
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
		mTable.CellSpacingX1 = inner_spacing_explicit + inner_spacing_for_border;
		mTable.CellSpacingX2 = inner_spacing_explicit;
		mTable.CellPaddingX = inner_padding_explicit;
		mTable.CellPaddingY = g.Style.CellPadding.y;

		const float outer_padding_for_border = (flags & ImGuiTableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
		const float outer_padding_explicit = pad_outer_x ? g.Style.CellPadding.x : 0.0f;
		mTable.OuterPaddingX = (outer_padding_for_border + outer_padding_explicit) - mTable.CellPaddingX;

		mTable.CurrentColumn = -1;
		mTable.CurrentRow = -1;
		mTable.RowBgColorCounter = 0;
		mTable.LastRowFlags = ImGuiTableRowFlags_None;
		mTable.InnerClipRect = (inner_window == outer_window) ? mTable.WorkRect : inner_window->ClipRect;
		mTable.InnerClipRect.ClipWith(mTable.WorkRect); // We need this to honor inner_width
		mTable.InnerClipRect.ClipWithFull(mTable.HostClipRect);
		mTable.InnerClipRect.Max.y = (flags & ImGuiTableFlags_NoHostExtendY) ? ImMin(mTable.InnerClipRect.Max.y, inner_window->WorkRect.Max.y) : inner_window->ClipRect.Max.y;

		mTable.RowPosY1 = mTable.RowPosY2 = mTable.WorkRect.Min.y; // This is needed somehow
		mTable.RowTextBaseline = 0.0f;                             // This will be cleared again by TableBeginRow()
		mTable.FreezeRowsRequest = mTable.FreezeRowsCount = 0;     // This will be setup by TableSetupScrollFreeze(), if any
		mTable.FreezeColumnsRequest = mTable.FreezeColumnsCount = 0;
		mTable.IsUnfrozenRows = true;
		mTable.DeclColumnsCount = 0;

		// Using opaque colors facilitate overlapping elements of the grid
		mTable.BorderColorStrong = ImGui::GetColorU32(ImGuiCol_TableBorderStrong);
		mTable.BorderColorLight = ImGui::GetColorU32(ImGuiCol_TableBorderLight);

		// Make table current
		// const int table_idx = g.Tables.GetIndex(mTable);
		// g.CurrentTableStack.push_back(ImGuiPtrOrIndex(table_idx));
		// g.CurrentTable = &mTable;
		// outer_window->DC.CurrentTableIdx = table_idx;
		// if (inner_window != outer_window) // So EndChild() within the inner window can restore the table properly.
		// inner_window->DC.CurrentTableIdx = table_idx;

		if ((table_last_flags & ImGuiTableFlags_Reorderable) && (flags & ImGuiTableFlags_Reorderable) == 0)
			mTable.IsResetDisplayOrderRequest = true;

		// Mark as used
		// if (table_idx >= g.TablesLastTimeActive.Size)
		//     g.TablesLastTimeActive.resize(table_idx + 1, -1.0f);
		// g.TablesLastTimeActive[table_idx] = (float)g.Time;
		mTable.MemoryCompacted = false;

		// Setup memory buffer (clear data if columns count changed)
		const int stored_size = mTable.Columns.size();
		if (stored_size != 0 && stored_size != columns_count) {
			IM_FREE(mTable.RawData);
			mTable.RawData = NULL;
		}
		if (mTable.RawData == NULL) {
			BeginInitMemory(columns_count);
			mTable.IsInitializing = mTable.IsSettingsRequestLoad = true;
		}
		if (mTable.IsResetAllRequest)
			ResetSettings();
		if (mTable.IsInitializing) {
			// Initialize
			mTable.SettingsOffset = -1;
			mTable.IsSortSpecsDirty = true;
			mTable.InstanceInteracted = -1;
			mTable.ContextPopupColumn = -1;
			mTable.ReorderColumn = mTable.ResizedColumn = mTable.LastResizedColumn = -1;
			mTable.AutoFitSingleColumn = -1;
			mTable.HoveredColumnBody = mTable.HoveredColumnBorder = -1;
			for (int n = 0; n < columns_count; n++) {
				TableColumn* column = &mTable.Columns[n];
				float width_auto = column->WidthAuto;
				*column = TableColumn();
				column->WidthAuto = width_auto;
				column->IsPreserveWidthAuto = true; // Preserve WidthAuto when reinitializing a live table: not technically necessary but remove a visible flicker
				column->DisplayOrder = mTable.DisplayOrderToIndex[n] = (TableColumnIdx) n;
				column->IsEnabled = column->IsEnabledNextFrame = true;
			}
		}

		// Load settings
		if (mTable.IsSettingsRequestLoad)
			LoadSettingsCustom();

		if (getCustomColumnsActive()) {
			bool expected = true;
			if (mSpecificColumnsUpdate.compare_exchange_strong(expected, false)) {
				if (mSpecificColumnsActive) {
					ApplySpecificColumnSetup();
				} else {
					LoadSettingsCustom();
				}
			}
		} else if (mSpecificColumnsActive) {
			LoadSettingsCustom();
			mSpecificColumnsActive = false;
		}

		// Handle DPI/font resize
		// This is designed to facilitate DPI changes with the assumption that e.g. style.CellPadding has been scaled as well.
		// It will also react to changing fonts with mixed results. It doesn't need to be perfect but merely provide a decent transition.
		// FIXME-DPI: Provide consistent standards for reference size. Perhaps using g.CurrentDpiScale would be more self explanatory.
		// This is will lead us to non-rounded WidthRequest in columns, which should work but is a poorly tested path.
		const float new_ref_scale_unit = g.FontSize; // g.Font->GetCharAdvance('A') ?
		if (mTable.RefScale != 0.0f && mTable.RefScale != new_ref_scale_unit) {
			const float scale_factor = new_ref_scale_unit / mTable.RefScale;
			//IMGUI_DEBUG_LOG("[table] %08X RefScaleUnit %.3f -> %.3f, scaling width by %.3f\n", mTable.ID, mTable.RefScaleUnit, new_ref_scale_unit, scale_factor);
			for (int n = 0; n < columns_count; n++)
				mTable.Columns[n].WidthRequest = mTable.Columns[n].WidthRequest * scale_factor;
		}
		mTable.RefScale = new_ref_scale_unit;

		// Disable output until user calls TableNextRow() or TableNextColumn() leading to the TableUpdateLayout() call..
		// This is not strictly necessary but will reduce cases were "out of table" output will be misleading to the user.
		// Because we cannot safely assert in EndTable() when no rows have been created, this seems like our best option.
		inner_window->SkipItems = true;

		// Clear names
		// At this point the ->NameOffset field of each column will be invalid until TableUpdateLayout() or the first call to TableSetupColumn()
		if (mTable.ColumnsNames.Buf.Size > 0)
			mTable.ColumnsNames.Buf.resize(0);

		// Apply queued resizing/reordering/hiding requests
		BeginApplyRequests();

		return true;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::End() {
		ImGuiContext& g = *GImGui;

		// This assert would be very useful to catch a common error... unfortunately it would probably trigger in some
		// cases, and for consistency user may sometimes output empty tables (and still benefit from e.g. outer border)
		//IM_ASSERT(table->IsLayoutLocked && "Table unused: never called TableNextRow(), is that the intent?");

		// If the user never got to call TableNextRow() or TableNextColumn(), we call layout ourselves to ensure all our
		// code paths are consistent (instead of just hoping that TableBegin/TableEnd will work), get borders drawn, etc.
		if (!mTable.IsLayoutLocked)
			UpdateLayout();

		const ImGuiTableFlags flags = mTable.Flags;
		ImGuiWindow* inner_window = mTable.InnerWindow;
		ImGuiWindow* outer_window = mTable.OuterWindow;
		IM_ASSERT(inner_window == g.CurrentWindow);
		IM_ASSERT(outer_window == inner_window || outer_window == inner_window->ParentWindow);

		if (mTable.IsInsideRow)
			EndRow();

		// Context menu in columns body
		// if (flags & ImGuiTableFlags_ContextMenuInBody)
		//     if (mTable.HoveredColumnBody != -1 && !IsAnyItemHovered() && IsMouseReleased(ImGuiMouseButton_Right))
		//         OpenContextMenu((int)mTable.HoveredColumnBody);

		// Finalize table height
		inner_window->DC.PrevLineSize = mTable.HostBackupPrevLineSize;
		inner_window->DC.CurrLineSize = mTable.HostBackupCurrLineSize;
		inner_window->DC.CursorMaxPos = mTable.HostBackupCursorMaxPos;
		const float inner_content_max_y = mTable.RowPosY2;
		IM_ASSERT(mTable.RowPosY2 == inner_window->DC.CursorPos.y);
		if (inner_window != outer_window)
			inner_window->DC.CursorMaxPos.y = inner_content_max_y;
		else if (!(flags & ImGuiTableFlags_NoHostExtendY))
			mTable.OuterRect.Max.y = mTable.InnerRect.Max.y = ImMax(mTable.OuterRect.Max.y, inner_content_max_y); // Patch OuterRect/InnerRect height
		mTable.WorkRect.Max.y = ImMax(mTable.WorkRect.Max.y, mTable.OuterRect.Max.y);
		mTable.LastOuterHeight = mTable.OuterRect.GetHeight();

		// Setup inner scrolling range
		// FIXME: This ideally should be done earlier, in BeginTable() SetNextWindowContentSize call, just like writing to inner_window->DC.CursorMaxPos.y,
		// but since the later is likely to be impossible to do we'd rather update both axises together.
		if (mTable.Flags & ImGuiTableFlags_ScrollX) {
			const float outer_padding_for_border = (mTable.Flags & ImGuiTableFlags_BordersOuterV) ? TABLE_BORDER_SIZE : 0.0f;
			float max_pos_x = mTable.InnerWindow->DC.CursorMaxPos.x;
			if (mTable.RightMostEnabledColumn != -1)
				max_pos_x = ImMax(max_pos_x, mTable.Columns[mTable.RightMostEnabledColumn].WorkMaxX + mTable.CellPaddingX + mTable.OuterPaddingX - outer_padding_for_border);
			if (mTable.ResizedColumn != -1)
				max_pos_x = ImMax(max_pos_x, mTable.ResizeLockMinContentsX2);
			mTable.InnerWindow->DC.CursorMaxPos.x = max_pos_x;
		}

		// Pop clipping rect
		if (!(flags & ImGuiTableFlags_NoClip))
			inner_window->DrawList->PopClipRect();
		inner_window->ClipRect = inner_window->DrawList->_ClipRectStack.back();

		// Draw borders
		if ((flags & ImGuiTableFlags_Borders) != 0)
			DrawBorders();

		// Flatten channels and merge draw calls
		mTable.DrawSplitter.SetCurrentChannel(inner_window->DrawList, 0);
		if ((mTable.Flags & ImGuiTableFlags_NoClip) == 0)
			MergeDrawChannels();
		mTable.DrawSplitter.Merge(inner_window->DrawList);

		// Update ColumnsAutoFitWidth to get us ahead for host using our size to auto-resize without waiting for next BeginTable()
		const float width_spacings = (mTable.OuterPaddingX * 2.0f) + (mTable.CellSpacingX1 + mTable.CellSpacingX2) * (mTable.ColumnsEnabledCount - 1);
		mTable.ColumnsAutoFitWidth = width_spacings + (mTable.CellPaddingX * 2.0f) * mTable.ColumnsEnabledCount;
		for (int column_n = 0; column_n < mTable.ColumnsCount; column_n++)
			if (mTable.EnabledMaskByIndex.test(column_n)) {
				TableColumn* column = &mTable.Columns[column_n];
				if ((column->Flags & ImGuiTableColumnFlags_WidthFixed) && !(column->Flags & ImGuiTableColumnFlags_NoResize))
					mTable.ColumnsAutoFitWidth += column->WidthRequest;
				else
					mTable.ColumnsAutoFitWidth += GetColumnWidthAuto(column);
			}

		// Update scroll
		if ((mTable.Flags & ImGuiTableFlags_ScrollX) == 0 && inner_window != outer_window) {
			inner_window->Scroll.x = 0.0f;
		} else if (mTable.LastResizedColumn != -1 && mTable.ResizedColumn == -1 && inner_window->ScrollbarX && mTable.InstanceInteracted == mTable.InstanceCurrent) {
			// When releasing a column being resized, scroll to keep the resulting column in sight
			const float neighbor_width_to_keep_visible = mTable.MinColumnWidth + mTable.CellPaddingX * 2.0f;
			TableColumn* column = &mTable.Columns[mTable.LastResizedColumn];
			if (column->MaxX < mTable.InnerClipRect.Min.x)
				ImGui::SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x - neighbor_width_to_keep_visible, 1.0f);
			else if (column->MaxX > mTable.InnerClipRect.Max.x)
				ImGui::SetScrollFromPosX(inner_window, column->MaxX - inner_window->Pos.x + neighbor_width_to_keep_visible, 1.0f);
		}

		// Apply resizing/dragging at the end of the frame
		if (mTable.ResizedColumn != -1 && mTable.InstanceCurrent == mTable.InstanceInteracted) {
			TableColumn* column = &mTable.Columns[mTable.ResizedColumn];
			const float new_x2 = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + TABLE_RESIZE_SEPARATOR_HALF_THICKNESS);
			const float new_width = ImFloor(new_x2 - column->MinX - mTable.CellSpacingX1 - mTable.CellPaddingX * 2.0f);
			mTable.ResizedColumnNextWidth = new_width;
		}

		// Pop from id stack
		IM_ASSERT_USER_ERROR(inner_window->IDStack.back() == mTable.ID + mTable.InstanceCurrent, "Mismatching PushID/PopID!");
		IM_ASSERT_USER_ERROR(outer_window->DC.ItemWidthStack.Size >= mTable.HostBackupItemWidthStackSize, "Too many PopItemWidth!");
		ImGui::PopID();

		// Restore window data that we modified
		const ImVec2 backup_outer_max_pos = outer_window->DC.CursorMaxPos;
		inner_window->WorkRect = mTable.HostBackupWorkRect;
		inner_window->ParentWorkRect = mTable.HostBackupParentWorkRect;
		inner_window->SkipItems = mTable.HostSkipItems;
		outer_window->DC.CursorPos = mTable.OuterRect.Min;
		outer_window->DC.ItemWidth = mTable.HostBackupItemWidth;
		outer_window->DC.ItemWidthStack.Size = mTable.HostBackupItemWidthStackSize;
		outer_window->DC.ColumnsOffset = mTable.HostBackupColumnsOffset;

		// Layout in outer window
		// (FIXME: To allow auto-fit and allow desirable effect of SameLine() we dissociate 'used' vs 'ideal' size by overriding
		// CursorPosPrevLine and CursorMaxPos manually. That should be a more general layout feature, see same problem e.g. #3414)
		if (inner_window != outer_window) {
			ImGui::EndChild();
		} else {
			ImGui::ItemSize(mTable.OuterRect.GetSize());
			ImGui::ItemAdd(mTable.OuterRect, 0);
		}

		// Override declared contents width/height to enable auto-resize while not needlessly adding a scrollbar
		if (mTable.Flags & ImGuiTableFlags_NoHostExtendX) {
			// FIXME-TABLE: Could we remove this section?
			// ColumnsAutoFitWidth may be one frame ahead here since for Fixed+NoResize is calculated from latest contents
			IM_ASSERT((mTable.Flags & ImGuiTableFlags_ScrollX) == 0);
			outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, mTable.OuterRect.Min.x + mTable.ColumnsAutoFitWidth);
		} else if (mTable.UserOuterSize.x <= 0.0f) {
			const float decoration_size = (mTable.Flags & ImGuiTableFlags_ScrollX) ? inner_window->ScrollbarSizes.x : 0.0f;
			outer_window->DC.IdealMaxPos.x = ImMax(outer_window->DC.IdealMaxPos.x, mTable.OuterRect.Min.x + mTable.ColumnsAutoFitWidth + decoration_size - mTable.UserOuterSize.x);
			outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, ImMin(mTable.OuterRect.Max.x, mTable.OuterRect.Min.x + mTable.ColumnsAutoFitWidth));
		} else {
			outer_window->DC.CursorMaxPos.x = ImMax(backup_outer_max_pos.x, mTable.OuterRect.Max.x);
		}
		if (mTable.UserOuterSize.y <= 0.0f) {
			const float decoration_size = (mTable.Flags & ImGuiTableFlags_ScrollY) ? inner_window->ScrollbarSizes.y : 0.0f;
			outer_window->DC.IdealMaxPos.y = ImMax(outer_window->DC.IdealMaxPos.y, inner_content_max_y + decoration_size - mTable.UserOuterSize.y);
			outer_window->DC.CursorMaxPos.y = ImMax(backup_outer_max_pos.y, ImMin(mTable.OuterRect.Max.y, inner_content_max_y));
		} else {
			// OuterRect.Max.y may already have been pushed downward from the initial value (unless ImGuiTableFlags_NoHostExtendY is set)
			outer_window->DC.CursorMaxPos.y = ImMax(backup_outer_max_pos.y, mTable.OuterRect.Max.y);
		}

		// Save settings
		if (mTable.IsSettingsDirty)
			SaveSettingsCustom();
		mTable.IsInitializing = false;

		// Clear or restore current table, if any
		// IM_ASSERT(g.CurrentWindow == outer_window && g.CurrentTable == mTable);
		// g.CurrentTableStack.pop_back();
		// g.CurrentTable = g.CurrentTableStack.Size ? g.Tables.GetByIndex(g.CurrentTableStack.back().Index) : NULL;
		// outer_window->DC.CurrentTableIdx = g.CurrentTable ? g.Tables.GetIndex(g.CurrentTable) : -1;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::MenuItemColumnVisibility(int TableColumnIdx) {
		TableColumn& column = mTable.Columns[TableColumnIdx];
		const char* columnName = GetColumnName(TableColumnIdx);
		// Make sure we can't hide the last active column
		bool menu_item_active = (column.Flags & ImGuiTableColumnFlags_NoHide) ? false : true;
		if (column.IsEnabled && mTable.ColumnsEnabledCount <= 1)
			menu_item_active = false;
		if (ImGui::MenuItem(columnName, NULL, column.IsEnabled, menu_item_active && !mSpecificColumnsActive))
			column.IsEnabledNextFrame = !column.IsEnabled;

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", mColumns.at(TableColumnIdx).Popup().c_str());
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	const char* MainTable<MaxColumnCount>::GetColumnName(int column_n) {
		if (mTable.IsLayoutLocked == false && column_n >= mTable.DeclColumnsCount)
			return ""; // NameOffset is invalid at this point
		const TableColumn* column = &mTable.Columns[column_n];
		if (column->NameOffset == -1)
			return "";
		return &mTable.ColumnsNames.Buf[column->NameOffset];
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetupScrollFreeze(int columns, int rows) {
		IM_ASSERT(mTable.IsLayoutLocked == false && "Need to call TableSetupColumn() before first row!");
		IM_ASSERT(columns >= 0 && columns < IMGUI_TABLE_MAX_COLUMNS);
		IM_ASSERT(rows >= 0 && rows < 128); // Arbitrary limit

		mTable.FreezeColumnsRequest = (mTable.Flags & ImGuiTableFlags_ScrollX) ? (TableColumnIdx) columns : 0;
		mTable.FreezeColumnsCount = (mTable.InnerWindow->Scroll.x != 0.0f) ? mTable.FreezeColumnsRequest : 0;
		mTable.FreezeRowsRequest = (mTable.Flags & ImGuiTableFlags_ScrollY) ? (TableColumnIdx) rows : 0;
		mTable.FreezeRowsCount = (mTable.InnerWindow->Scroll.y != 0.0f) ? mTable.FreezeRowsRequest : 0;
		mTable.IsUnfrozenRows = (mTable.FreezeRowsCount == 0); // Make sure this is set before TableUpdateLayout() so ImGuiListClipper can benefit from it.b
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	ImRect MainTable<MaxColumnCount>::GetCellBgRect(int column_n) {
		const TableColumn* column = &mTable.Columns[column_n];
		float x1 = column->MinX;
		float x2 = column->MaxX;
		if (column->PrevEnabledColumn == -1)
			x1 -= mTable.CellSpacingX1;
		if (column->NextEnabledColumn == -1)
			x2 += mTable.CellSpacingX2;
		return ImRect(x1, mTable.RowPosY1, x2, mTable.RowPosY2);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	ImGuiTableSortSpecs* MainTable<MaxColumnCount>::GetSortSpecs() {
		if (!(mTable.Flags & ImGuiTableFlags_Sortable))
			return NULL;

		// Require layout (in case TableHeadersRow() hasn't been called) as it may alter IsSortSpecsDirty in some paths.
		if (!mTable.IsLayoutLocked)
			UpdateLayout();

		if (mTable.IsSortSpecsDirty)
			SortSpecsBuild();

		return &mTable.SortSpecs;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::SetupColumn(const char* label, ImGuiTableColumnFlags flags, float init_width_or_weight, ImGuiID user_id) {
		IM_ASSERT(mTable.IsLayoutLocked == false && "Need to call call TableSetupColumn() before first row!");
		IM_ASSERT((flags & ImGuiTableColumnFlags_StatusMask_) == 0 && "Illegal to pass StatusMask values to TableSetupColumn()");
		if (mTable.DeclColumnsCount >= mTable.ColumnsCount) {
			IM_ASSERT_USER_ERROR(mTable.DeclColumnsCount < mTable.ColumnsCount, "Called TableSetupColumn() too many times!");
			return;
		}

		TableColumn* column = &mTable.Columns[mTable.DeclColumnsCount];
		mTable.DeclColumnsCount++;

		// Assert when passing a width or weight if policy is entirely left to default, to avoid storing width into weight and vice-versa.
		// Give a grace to users of ImGuiTableFlags_ScrollX.
		if (mTable.IsDefaultSizingPolicy && (flags & ImGuiTableColumnFlags_WidthMask_) == 0 && (flags & ImGuiTableFlags_ScrollX) == 0)
			IM_ASSERT(init_width_or_weight <= 0.0f && "Can only specify width/weight if sizing policy is set explicitely in either Table or Column.");

		// When passing a width automatically enforce WidthFixed policy
		// (whereas TableSetupColumnFlags would default to WidthAuto if mTable.s not Resizable)
		if ((flags & ImGuiTableColumnFlags_WidthMask_) == 0 && init_width_or_weight > 0.0f)
			if ((mTable.Flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedFit || (mTable.Flags & ImGuiTableFlags_SizingMask_) == ImGuiTableFlags_SizingFixedSame)
				flags |= ImGuiTableColumnFlags_WidthFixed;

		TableSetupColumnFlags(column, flags);

		column->UserID = user_id;
		flags = column->Flags;

		// Initialize defaults
		column->InitStretchWeightOrWidth = init_width_or_weight;
		if (mTable.IsInitializing) {
			// Init width or weight
			if (column->WidthRequest < 0.0f && column->StretchWeight < 0.0f) {
				if ((flags & ImGuiTableColumnFlags_WidthFixed) && init_width_or_weight > 0.0f)
					column->WidthRequest = init_width_or_weight;
				if (flags & ImGuiTableColumnFlags_WidthStretch)
					column->StretchWeight = (init_width_or_weight > 0.0f) ? init_width_or_weight : -1.0f;

				// Disable auto-fit if an explicit width/weight has been specified
				if (init_width_or_weight > 0.0f)
					column->AutoFitQueue = 0x00;
			}

			// Init default visibility/sort state
			if ((flags & ImGuiTableColumnFlags_DefaultHide) && (mTable.SettingsLoadedFlags & ImGuiTableFlags_Hideable) == 0)
				column->IsEnabled = column->IsEnabledNextFrame = false;
			if (flags & ImGuiTableColumnFlags_DefaultSort && (mTable.SettingsLoadedFlags & ImGuiTableFlags_Sortable) == 0) {
				column->SortOrder = 0; // Multiple columns using _DefaultSort will be reassigned unique SortOrder values when building the sort specs.
				column->SortDirection = (column->Flags & ImGuiTableColumnFlags_PreferSortDescending) ? (ImS8) ImGuiSortDirection_Descending : (ImU8) (ImGuiSortDirection_Ascending);
			}
		}

		// Store name (append with zero-terminator in contiguous buffer)
		column->NameOffset = -1;
		if (label != NULL && label[0] != 0) {
			column->NameOffset = (ImS16) mTable.ColumnsNames.size();
			mTable.ColumnsNames.append(label, label + strlen(label) + 1);
		}
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::ColumnHeader(const char* label, bool show_label, ImTextureID texture, Alignment alignment, const char* popupText) {
		// TODO change eventually (to line height or something)
		const float image_size = 16.f;

		// Show label if texture is null
		if (!texture || getShowHeaderAsText()) {
			show_label = true;
		}

		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		if (window->SkipItems)
			return;

		IM_ASSERT(mTable.CurrentColumn != -1);
		const int column_n = mTable.CurrentColumn;
		TableColumn* column = &mTable.Columns[column_n];

		// Label
		if (label == NULL)
			label = "";
		const char* label_end = ImGui::FindRenderedTextEnd(label);
		ImVec2 label_size = ImGui::CalcTextSize(label, label_end, true);
		ImVec2 label_pos = window->DC.CursorPos;

		// If we already got a row height, there's use that.
		// FIXME-TABLE: Padding problem if the correct outer-padding CellBgRect strays off our ClipRect?
		ImRect cell_r = GetCellBgRect(column_n);
		float label_height = mTable.RowMinHeight - mTable.CellPaddingY * 2.0f;
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
		if ((mTable.Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
			if (column->SortOrder > -1) {
				w_arrow = ImFloor(g.FontSize * ARROW_SCALE + g.Style.FramePadding.x);
				if (column->SortOrder > 0) {
					ImFormatString(sort_order_suf, IM_ARRAYSIZE(sort_order_suf), "%d", column->SortOrder + 1);
					w_sort_text = g.Style.ItemInnerSpacing.x + ImGui::CalcTextSize(sort_order_suf).x;
				}
			} else {
				w_arrow = g.Style.ItemInnerSpacing.x;
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
		const bool selected = (mTable.IsContextPopupOpen && mTable.ContextPopupColumn == column_n && mTable.InstanceInteracted == mTable.InstanceCurrent);
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
			SetBgColor(ImGuiTableBgTarget_CellBg, col, mTable.CurrentColumn);
			ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
		} else {
			// Submit single cell bg color in the case we didn't submit a full header row
			if ((mTable.RowFlags & ImGuiTableRowFlags_Headers) == 0)
				SetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg), mTable.CurrentColumn);
		}
		if (held)
			mTable.HeldHeaderColumn = (TableColumnIdx) column_n;
		window->DC.CursorPos.y -= g.Style.ItemSpacing.y * 0.5f;

		// Drag and drop to re-order columns.
		// FIXME-TABLE: Scroll request while reordering a column and it lands out of the scrolling zone.
		if (held && (mTable.Flags & ImGuiTableFlags_Reorderable) && ImGui::IsMouseDragging(0) && !g.DragDropActive) {
			// While moving a column it will jump on the other side of the mouse, so we also test for MouseDelta.x
			mTable.ReorderColumn = (TableColumnIdx) column_n;
			mTable.InstanceInteracted = mTable.InstanceCurrent;

			// We don't reorder: through the frozen<>unfrozen line, or through a column that is marked with ImGuiTableColumnFlags_NoReorder.
			if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < cell_r.Min.x)
				if (TableColumn* prev_column = (column->PrevEnabledColumn != -1) ? &mTable.Columns[column->PrevEnabledColumn] : NULL)
					if (!((column->Flags | prev_column->Flags) & ImGuiTableColumnFlags_NoReorder))
						if ((column->IndexWithinEnabledSet < mTable.FreezeColumnsRequest) == (prev_column->IndexWithinEnabledSet < mTable.FreezeColumnsRequest))
							mTable.ReorderColumnDir = -1;
			if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > cell_r.Max.x)
				if (TableColumn* next_column = (column->NextEnabledColumn != -1) ? &mTable.Columns[column->NextEnabledColumn] : NULL)
					if (!((column->Flags | next_column->Flags) & ImGuiTableColumnFlags_NoReorder))
						if ((column->IndexWithinEnabledSet < mTable.FreezeColumnsRequest) == (next_column->IndexWithinEnabledSet < mTable.FreezeColumnsRequest))
							mTable.ReorderColumnDir = +1;
		}

		// Sort order arrow
		const float ellipsis_max = cell_r.Max.x - w_arrow - w_sort_text;
		if ((mTable.Flags & ImGuiTableFlags_Sortable) && !(column->Flags & ImGuiTableColumnFlags_NoSort)) {
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
			if (pressed && mTable.ReorderColumn != column_n) {
				ImGuiSortDirection sort_direction = GetColumnNextSortDirection(column);
				SetColumnSortDirection(column_n, sort_direction, g.IO.KeyShift);
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
					newX = ellipsis_max - ((ellipsis_max - label_pos.x) / 2) - (label_size.x / 2);

					if (newX <= label_pos.x) newX = label_pos.x;
					break;
				case Alignment::Right:
					newX = ellipsis_max - label_size.x;
					if (newX <= label_pos.x) newX = label_pos.x;
					break;
				// Nothing to do
				case Alignment::Left:
				case Alignment::Unaligned: break;
			}

			ImGui::RenderTextEllipsis(window->DrawList, ImVec2(newX, label_pos.y), ImVec2(ellipsis_max, label_pos.y + label_height + g.Style.FramePadding.y), ellipsis_max, ellipsis_max, label, label_end, &label_size);
		} else {
			float newX = label_pos.x;

			switch (alignment) {
				case Alignment::Center:
					newX = ellipsis_max - ((ellipsis_max - label_pos.x) / 2) - (image_size / 2);
					if (newX <= label_pos.x) newX = label_pos.x;
					break;
				case Alignment::Right:
					newX = ellipsis_max - image_size;
					// ImGui::SetCursorPosX(cursorPosX + textSpace - contentSize.x);
					if (newX <= label_pos.x) newX = label_pos.x;
					break;
				// Nothing to do
				case Alignment::Left:
				case Alignment::Unaligned: break;
			}

			ImRect ibb(ImVec2(newX, label_pos.y), ImVec2(newX, label_pos.y) + image_size);

			window->DrawList->AddImage(texture, ibb.Min, ibb.Max);
		}

		// const bool text_clipped = label_size.x > (ellipsis_max - label_pos.x);
		// if (text_clipped && hovered && g.HoveredIdNotActiveTimer > g.TooltipSlowDelay)
		// ImGui::SetTooltip("%.*s", (int)(label_end - label), label);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", popupText);
		}

		// We don't use BeginPopupContextItem() because we want the popup to stay up even after the column is hidden
		// if (ImGui::IsMouseReleased(1) && ImGui::IsItemHovered())
		// ImGui::TableOpenContextMenu(column_n);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::AlignedTextColumn(const char* text) {
		const float posX = ImGui::GetCursorPosX();
		float newX = posX;
		float textWidth = ImGui::CalcTextSize(text).x;
		ImGuiWindow* window = ImGui::GetCurrentWindowRead();
		float columnWidth = window->WorkRect.Max.x - window->DC.CursorPos.x;

		switch (getAlignment()) {
			case Alignment::Left:
				break;
			case Alignment::Center:
				newX = posX + columnWidth / 2 - textWidth / 2;
				break;
			case Alignment::Right:
				newX = posX + columnWidth - textWidth;
				break;
			// Nothing to do
			case Alignment::Unaligned: break;
		}

		// Clip to left, if text is bigger than current column
		if (newX < posX) {
			newX = posX;
		}

		ImGui::SetCursorPosX(newX);

		ImGui::TextUnformatted(text);

		// Set cursorMaxPos manually, so right aligned elements also shrink the column
		window->DC.CursorMaxPos.x = window->Pos.x + posX + textWidth;
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::AlignedTextColumn(const std::string& text) {
		AlignedTextColumn(text.c_str());
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	template<typename... Args>
	void MainTable<MaxColumnCount>::AlignedTextColumn(std::string_view format, Args&&... args) {
		AlignedTextColumn(std::vformat(format, std::make_format_args(args...)));
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	bool MainTable<MaxColumnCount>::SpinnerAligned(const char* label, float radius, float thickness, const ImU32& color) {
		const float posX = ImGui::GetCursorPosX();
		float newX = posX;
		float elementWidth = radius * 2 + thickness * 2;
		ImGuiWindow* window = ImGui::GetCurrentWindowRead();
		float columnWidth = window->WorkRect.Max.x - window->DC.CursorPos.x;

		switch (getAlignment()) {
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

		return ImGuiEx::Spinner(label, radius, thickness, color);
	}

	template<size_t MaxColumnCount>
	requires SmallerThanMaxColumnAmount<MaxColumnCount>
	void MainTable<MaxColumnCount>::ApplySpecificColumnSetup() {
		for (int i = getCustomColumnsFirstColumn(); i < mTable.Columns.size(); ++i) {
			auto& column = mTable.Columns[i];
			bool enabled = std::ranges::find_if(mSpecificColumnCache, [i, this](const size_t& pVal) -> bool { return mColumns.at(i).UserId == pVal; }) != mSpecificColumnCache.end();
			column.IsEnabledNextFrame = enabled;
		}
	}
} // namespace ArcdpsExtension

#pragma warning(pop)
