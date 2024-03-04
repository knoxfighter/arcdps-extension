#pragma once
#include "arcdps_structs_slim.h"

#include <string>
#include <Windows.h>

bool is_player(const ag* new_player);

typedef UINT (*WindowCallbackSignature)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef void (*CombatCallbackSignature)(cbtevent* ev, ag* src, ag* dst, const char* skillname, uint64_t id, uint64_t revision);
typedef void (*ImguiCallbackSignature)(uint32_t not_charsel_or_loading, uint32_t hide_if_combat_or_ooc);
typedef void (*OptionsEndCallbackSignature)();
typedef void (*OptionsWindowsCallbackSignature)(const char* windowname);

typedef struct arcdps_exports {
	uintptr_t size;                                  /* size of exports table */
	uint32_t sig;                                    /* pick a number between 0 and uint32_t max that isn't used by other modules */
	uint32_t imguivers;                              /* set this to IMGUI_VERSION_NUM. if you don't use imgui, 18000 (as of 2021-02-02) */
	const char* out_name;                            /* name string */
	const char* out_build;                           /* build string */
	WindowCallbackSignature wnd_nofilter;            /* wndproc callback, fn(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) */
	CombatCallbackSignature combat;                  /* combat event callback, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
	ImguiCallbackSignature imgui;                    /* id3dd9::present callback, before imgui::render, fn(uint32_t not_charsel_or_loading) */
	OptionsEndCallbackSignature options_end;         /* id3dd9::present callback, appending to the end of options window in arcdps, fn() */
	CombatCallbackSignature combat_local;            /* combat event callback like area but from chat log, fn(cbtevent* ev, ag* src, ag* dst, char* skillname, uint64_t id, uint64_t revision) */
	WindowCallbackSignature wnd_filter;              /* wndproc callback like above, input filered using modifiers */
	OptionsWindowsCallbackSignature options_windows; /* called once per 'window' option checkbox, with null at the end, non-zero return disables drawing that option, fn(char* windowname) */
} arcdps_exports;
static_assert(sizeof(arcdps_exports) == 88);

typedef void* (*MallocSignature)(size_t);
typedef void (*FreeSignature)(void*);

typedef arcdps_exports* (*ModInitSignature)();
typedef uintptr_t (*ModReleaseSignature)();

struct ID3D11Device;
struct ImGuiContext;

/**
 * `dxver`: The used directx version (either 9 or 11)
 * `dxptr`: The pointer to the directx context. For dx9 `IDirect3DDevice9*`. For dx11 `IDXGISwapChain`.
 *
 * To get the Device from the Swapchain:
 * ```C++
 * ID3D11Device* pDevice;
 * g_pSwapChain->GetDevice( __uuidof(pDevice), (void**)&pDevice);
 * ```
 */
typedef ModInitSignature (*GetInitAddrSignature)(const char* arcversion, ImGuiContext* imguictx, void* dxptr, HMODULE arcdll, MallocSignature mallocfn, FreeSignature freefn, UINT dxver);
typedef ModReleaseSignature (*GetReleaseAddrSignature)();

// ARCPDS dll-exports
typedef uint64_t (*arc_export_func_u64)();
typedef void (*e3_func_ptr)(const char* str);

// Define these exports in your main.cpp
extern arc_export_func_u64 ARC_EXPORT_E6;
extern arc_export_func_u64 ARC_EXPORT_E7;
extern e3_func_ptr ARC_LOG_FILE;
extern e3_func_ptr ARC_LOG;

// additional enum for alignment
enum class Alignment {
	Left,
	Center,
	Right,
	Unaligned,
};

std::string to_string(Alignment alignment);

enum class Position {
	Manual,
	ScreenRelative,
	WindowRelative,
};

std::string to_string(Position position);

enum class CornerPosition {
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

std::string to_string(CornerPosition position);

enum class SizingPolicy {
	SizeToContent,
	SizeContentToWindow,
	ManualWindowSize,
};

std::string to_string(SizingPolicy sizingPolicy);
