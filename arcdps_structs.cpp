#include "arcdps_structs.h"

#ifndef ARCDPS_EXTENSION_NO_LANG_H
#include "ExtensionTranslations.h"
#include "Localization.h"
std::string to_string(Alignment alignment) {
	switch (alignment) {
		case Alignment::Left: return Localization::STranslate(ET_Left);
		case Alignment::Center: return Localization::STranslate(ET_Center);
		case Alignment::Right: return Localization::STranslate(ET_Right);
		case Alignment::Unaligned: return Localization::STranslate(ET_Unaligned);
		default: return "Unknown";
	}
}

std::string to_string(Position position) {
	switch (position) {
		case Position::Manual: return Localization::STranslate(ET_PositionManual);
		case Position::ScreenRelative: return Localization::STranslate(ET_PositionScreenRelative);
		case Position::WindowRelative: return Localization::STranslate(ET_PositionWindowRelative);
		default: return Localization::STranslate(ET_Unknown);
	}
}

std::string to_string(CornerPosition position) {
	switch (position) {
		case CornerPosition::TopLeft: return Localization::STranslate(ET_CornerPositionTopLeft);
		case CornerPosition::TopRight: return Localization::STranslate(ET_CornerPositionTopRight);
		case CornerPosition::BottomLeft: return Localization::STranslate(ET_CornerPositionBottomLeft);
		case CornerPosition::BottomRight: return Localization::STranslate(ET_CornerPositionBottomRight);
		default: return Localization::STranslate(ET_Unknown);
	}
}

std::string to_string(SizingPolicy sizingPolicy) {
	switch (sizingPolicy) {
		case SizingPolicy::SizeToContent: return Localization::STranslate(ET_SizingPolicySizeToContent);
		case SizingPolicy::SizeContentToWindow: return Localization::STranslate(ET_SizingPolicySizeContentToWindow);
		case SizingPolicy::ManualWindowSize: return Localization::STranslate(ET_SizingPolicyManualWindowSize);
		default: return Localization::STranslate(ET_Unknown);
	}
}
#else
std::string to_string(Alignment alignment) {
	switch (alignment) {
		case Alignment::Left: return "Left";
		case Alignment::Center: return "Center";
		case Alignment::Right: return "Right";
		case Alignment::Unaligned: return "Unaligned";
		default: return "Unknown";
	}
}

std::string to_string(Position position) {
	switch (position) {
		case Position::Manual: return "Manual";
		case Position::ScreenRelative: return "Screen Relative";
		case Position::WindowRelative: return "Window Relative";
		default: return "Unknown";
	}
}

std::string to_string(CornerPosition position) {
	switch (position) {
		case CornerPosition::TopLeft: return "Top-Left";
		case CornerPosition::TopRight: return "Top-Right";
		case CornerPosition::BottomLeft: return "Bottom-Left";
		case CornerPosition::BottomRight: return "Bottom-Right";
		default: return "Unknown";
	}
}

std::string to_string(SizingPolicy sizingPolicy) {
	switch (sizingPolicy) {
		case SizingPolicy::SizeToContent: return "Size to Content";
		case SizingPolicy::SizeContentToWindow: return "Size Content to Window";
		case SizingPolicy::ManualWindowSize: return "Manuel Window Size";
		default: return "Unknown";
	}
}
#endif

uint64_t arcExportDefaults() {
	return 0;
}

void e3Defaults(const char*) {}

arc_export_func_u64 ARC_EXPORT_E6 = arcExportDefaults;
arc_export_func_u64 ARC_EXPORT_E7 = arcExportDefaults;
e3_func_ptr ARC_LOG_FILE = e3Defaults;
e3_func_ptr ARC_LOG = e3Defaults;

bool is_player(const ag* new_player) {
	return new_player
		   && new_player->elite != 0xffffffff
		   && new_player->name
		   && std::string(new_player->name).length() > 1
		   && new_player->id;
}
