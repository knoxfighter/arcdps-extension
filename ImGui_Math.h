#pragma once

#if __has_include(<imgui/imgui.h>)
#include <imgui/imgui.h>
#else
#include "../imgui/imgui.h"
#endif

static inline ImVec2 operator+(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x + rhs, lhs.y + rhs); }
static inline ImVec4 operator*(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }
