#include "KeyInput.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#ifndef ARCDPS_EXTENSION_NO_LANG_H
#include "ExtensionTranslations.h"
#include "Localization.h"

#define POPUP_NAME_TEXT Localization::STranslate(ET_KeyInputPopupName)
#define APPLY_TEXT Localization::STranslate(ET_ApplyButton).data()
#define CANCEL_TEXT Localization::STranslate(ET_CancelButton).data()
#else
#define POPUP_NAME_TEXT "KeyBind"
#define APPLY_TEXT "Apply"
#define CANCEL_TEXT "Cancel"
#endif

namespace {
	using KeyBinds::KeyCode;
	constexpr KeyCode allowedKeyCodesValues[] = {
			KeyCode::LeftAlt, KeyCode::LeftCtrl, KeyCode::LeftShift, KeyCode::Quote, KeyCode::Hash,
			KeyCode::CapsLock, KeyCode::Colon, KeyCode::Minus, KeyCode::Equals, KeyCode::Escape,
			KeyCode::OpenBracket, KeyCode::NumLock, KeyCode::Period, KeyCode::CloseBracket, KeyCode::Semicolon,
			KeyCode::Slash, KeyCode::Print, KeyCode::Tilde, KeyCode::Backspace, KeyCode::Delete, KeyCode::Enter,
			KeyCode::Space, KeyCode::Tab, KeyCode::End, KeyCode::Home, KeyCode::Insert, KeyCode::Next,
			KeyCode::Prior, KeyCode::ArrowDown, KeyCode::ArrowLeft, KeyCode::ArrowRight, KeyCode::ArrowUp,
			KeyCode::F1, KeyCode::F2, KeyCode::F3, KeyCode::F4, KeyCode::F5, KeyCode::F6, KeyCode::F7, KeyCode::F8,
			KeyCode::F9, KeyCode::F10, KeyCode::F11, KeyCode::F12, KeyCode::_0, KeyCode::_1, KeyCode::_2,
			KeyCode::_3, KeyCode::_4, KeyCode::_5, KeyCode::_6, KeyCode::_7, KeyCode::_8, KeyCode::_9, KeyCode::A,
			KeyCode::B, KeyCode::C, KeyCode::D, KeyCode::E, KeyCode::F, KeyCode::G, KeyCode::H, KeyCode::I,
			KeyCode::J, KeyCode::K, KeyCode::L, KeyCode::M, KeyCode::N, KeyCode::O, KeyCode::P, KeyCode::Q,
			KeyCode::R, KeyCode::S, KeyCode::T, KeyCode::U, KeyCode::V, KeyCode::W, KeyCode::X, KeyCode::Y,
			KeyCode::Z, KeyCode::PlusNum, KeyCode::DecimalNum, KeyCode::DivideNum, KeyCode::MultiplyNum,
			KeyCode::_0_NUM, KeyCode::_1_NUM, KeyCode::_2_NUM, KeyCode::_3_NUM, KeyCode::_4_NUM, KeyCode::_5_NUM,
			KeyCode::_6_NUM, KeyCode::_7_NUM, KeyCode::_8_NUM, KeyCode::_9_NUM, KeyCode::EnterNum,
			KeyCode::MinusNum, KeyCode::ImeKey1, KeyCode::ImeKey2, KeyCode::RightAlt, KeyCode::RightCtrl,
			KeyCode::Backslash, KeyCode::F13, KeyCode::F14, KeyCode::F15, KeyCode::F16, KeyCode::F17, KeyCode::F18,
			KeyCode::F19, KeyCode::F20, KeyCode::F21, KeyCode::F22, KeyCode::F23, KeyCode::F24, KeyCode::F25,
			KeyCode::F26, KeyCode::F27, KeyCode::F28, KeyCode::F29, KeyCode::F30, KeyCode::F31, KeyCode::F32,
			KeyCode::F33, KeyCode::F34, KeyCode::F35, KeyCode::RightShift, KeyCode::Eject, KeyCode::EqualNum,
			KeyCode::ClearNum, KeyCode::LeftCmd, KeyCode::Function, KeyCode::RightCmd};
	static_assert(std::size(allowedKeyCodesValues) == KeyBinds::KEY_CODES_SIZE_GW2);
} // namespace

namespace ImGuiEx {
	using namespace ArcdpsExtension;

	static inline KeyBinds::Key keyCodeInputKeyState;
	static inline KeyBinds::Modifier keyCodeInputCurrentModifier;
	static inline bool keyCodeInputActive = false;
	static inline int keyCodeInputLastActiveFrame = 0;
	static inline KeyCodeInputFlags keyCodeInputFlags = 0;

	bool KeyCodeInputWndHandle(
			[[maybe_unused]] HWND pWindowHandle,
			UINT pMessage,
			[[maybe_unused]] WPARAM pAdditionalW,
			[[maybe_unused]] LPARAM pAdditionalL
	) {
		if (keyCodeInputActive == true) {
			// if not called last frame, the window is closed and we should not consume any events anymore
			if (keyCodeInputLastActiveFrame < ImGui::GetFrameCount() - 1) {
				keyCodeInputActive = false;
				return false;
			}

			switch (pMessage) {
				case WM_KEYUP:
				case WM_SYSKEYUP: {
					UINT scanCode = (pAdditionalL >> 16) & 0x00ff;
					const bool extended = (pAdditionalL >> 24) & 0x1;

					if (extended == true) {
						scanCode |= 0xE000;
					}

					const auto& keyCode = KeyBinds::MsvcScanCodeToKeyCode(scanCode);
					if (!keyCode) break;
					if (!(keyCodeInputFlags & KeyCodeInputFlags_NoModifier)) {
						KeyBinds::Modifier modifier = KeyBinds::GetModifier(keyCode.value());
						if (modifier != 0) {
							keyCodeInputCurrentModifier &= !modifier;
						}
					}
					return true;
				}
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN: {
					UINT scanCode = (pAdditionalL >> 16) & 0x00ff;
					const bool extended = (pAdditionalL >> 24) & 0x1;
					const bool previous = (pAdditionalL >> 30) & 0x1;

					if (previous == true) return true;

					if (extended == true) {
						scanCode |= 0xE000;
					}

					const auto& keyCodeOpt = KeyBinds::MsvcScanCodeToKeyCode(scanCode);
					if (!keyCodeOpt) break;
					const KeyBinds::KeyCode& keyCode = keyCodeOpt.value();

					if (keyCodeInputFlags & KeyCodeInputFlags_OnlyGW2Keys) {
						if (std::ranges::find(allowedKeyCodesValues, keyCode) == std::end(allowedKeyCodesValues)) {
							return true;
						}
					}

					keyCodeInputKeyState.DeviceType = KeyBinds::DeviceType::Keyboard;
					keyCodeInputKeyState.Code = static_cast<int32_t>(keyCode);
					if (!(keyCodeInputFlags & KeyCodeInputFlags_NoModifier)) {
						KeyBinds::Modifier modifier = KeyBinds::GetModifier(keyCode);
						if (modifier != 0) {
							keyCodeInputCurrentModifier |= modifier;
							keyCodeInputKeyState.Modifier = 0;
						} else {
							keyCodeInputKeyState.Modifier = keyCodeInputCurrentModifier;
						}
					}
					return true;
				}
				case WM_XBUTTONDOWN: {
					if (!(keyCodeInputFlags & KeyCodeInputFlags_NoMouse)) {
						WORD word = GET_XBUTTON_WPARAM(pAdditionalW);
						if (word == XBUTTON1) {
							keyCodeInputKeyState.DeviceType = KeyBinds::DeviceType::Mouse;
							keyCodeInputKeyState.Code = static_cast<int32_t>(KeyBinds::MouseCode::Mouse_4);
							keyCodeInputKeyState.Modifier = keyCodeInputCurrentModifier;
						} else if (word == XBUTTON2) {
							keyCodeInputKeyState.DeviceType = KeyBinds::DeviceType::Mouse;
							keyCodeInputKeyState.Code = static_cast<int32_t>(KeyBinds::MouseCode::Mouse_5);
							keyCodeInputKeyState.Modifier = keyCodeInputCurrentModifier;
						}
					}
					return true;
				}
				case WM_MBUTTONDOWN: {
					if (!(keyCodeInputFlags & KeyCodeInputFlags_NoMouse)) {
						keyCodeInputKeyState.DeviceType = KeyBinds::DeviceType::Mouse;
						keyCodeInputKeyState.Code = static_cast<int32_t>(KeyBinds::MouseCode::Mouse_3);
						keyCodeInputKeyState.Modifier = keyCodeInputCurrentModifier;
					}
					return true;
				}
				case WM_KILLFOCUS: {
					if (!(keyCodeInputFlags & KeyCodeInputFlags_NoMouse)) {
						// reset modifier when loosing focus (Alt+Tab)
						keyCodeInputCurrentModifier = 0;
					}
					return true;
				}
			}
		}
		return false;
	}

	void OpenKeyCodePopupState(const KeyBinds::Key& pKeyContainer, KeyCodeInputFlags pFlags, KeyBinds::Modifier pFixedModifier) {
		keyCodeInputActive = true;
		keyCodeInputKeyState = pKeyContainer;
		keyCodeInputKeyState.Modifier = pFixedModifier;
		keyCodeInputCurrentModifier = 0;
		keyCodeInputFlags = pFlags;
	}

	void CloseKeyCodePopupState() {
		keyCodeInputActive = false;
	}

	void RestartKeyCodePopupState() {
		keyCodeInputActive = true;
	}

	void KeyCodeInputActiveFrame() {
		keyCodeInputLastActiveFrame = ImGui::GetFrameCount();
	}

	const KeyBinds::Key& GetKeyCodeInputKeyState() {
		return keyCodeInputKeyState;
	}

	bool KeyCodeInput(const char* pLabel, KeyBinds::Key& pKeyContainer, Language pLanguage, HKL pHkl, KeyCodeInputFlags pFlags, KeyBinds::Modifier pFixedModifier) {
		bool res = false;

		pFlags |= (pFlags & KeyCodeInputFlags_FixedModifier) ? KeyCodeInputFlags_NoModifier : 0;

		KeyCodeInputActiveFrame();

		ImGui::TextUnformatted(pLabel);
		ImGui::SameLine();

		KeyBinds::Key keyContainerCopy = pKeyContainer;
		if (pFlags & KeyCodeInputFlags_FixedModifier) {
			keyContainerCopy.Modifier = pFixedModifier;
		}

		std::string keyStr = to_string(keyContainerCopy, pLanguage, pHkl);
		ImVec2 textSize = ImGui::CalcTextSize(keyStr.c_str());
		keyStr.append("##");
		keyStr.append(pLabel);

		std::string popupName = std::string(POPUP_NAME_TEXT);
		popupName.append(" - ");
		popupName.append(pLabel);

		if (ImGui::Button(keyStr.c_str(), ImVec2(textSize.x + 50.f, 0.f))) {
			ImGui::OpenPopup(popupName.c_str());

			OpenKeyCodePopupState(pKeyContainer, pFlags, pFixedModifier);
		}

		// Always center this window when appearing
		ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(center, 0 /*ImGuiCond_Appearing*/, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(550.f, 250.f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(350.f, FLT_MIN), ImVec2(FLT_MAX, FLT_MAX));
		if (ImGui::BeginPopupModal(popupName.c_str(), NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGuiStyle& style = ImGui::GetStyle();

			std::string keyCodeText = to_string(keyCodeInputKeyState, pLanguage, pHkl, true);
			ImGui::TextUnformatted(keyCodeText.c_str());
			ImGui::SameLine();

			const auto& stringUnbind = to_string_unbind(pLanguage);
			float buttonWidth = ImGui::CalcTextSize(stringUnbind.c_str()).x + style.FramePadding.x * 2;
			// ImGui::SameLine(ImGui::GetWindowWidth() - pos);
			float cursorPosX = ImGui::GetCursorPosX();
			float windowMaxX = ImGui::GetWindowContentRegionMax().x;
			float posX = windowMaxX - buttonWidth;
			if (posX < cursorPosX) posX = cursorPosX;
			ImGui::SetCursorPosX(posX);
			if (ImGui::Button(stringUnbind.c_str())) {
				keyCodeInputKeyState.Code = 0;
				keyCodeInputKeyState.DeviceType = KeyBinds::DeviceType::Unset;
				keyCodeInputKeyState.Modifier = 0;
			}
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			float cursorMaxPos = window->Pos.x + cursorPosX + buttonWidth;

			// ItemSpacing.x / 2, no idea why i have to /2 it ...
			float buttonSizeX = (ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x) / 2 - style.ItemSpacing.x / 2;
			const char* applyText = APPLY_TEXT;
			if (ImGui::Button(applyText, ImVec2(buttonSizeX, 0))) {
				pKeyContainer.DeviceType = keyCodeInputKeyState.DeviceType;
				pKeyContainer.Code = keyCodeInputKeyState.Code;
				pKeyContainer.Modifier = pFlags & KeyCodeInputFlags_FixedModifier ? 0 : keyCodeInputKeyState.Modifier;
				CloseKeyCodePopupState();
				ImGui::CloseCurrentPopup();
				res = true;
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			const char* cancelText = CANCEL_TEXT;
			if (ImGui::Button(cancelText, ImVec2(buttonSizeX, 0))) {
				CloseKeyCodePopupState();
				ImGui::CloseCurrentPopup();
			}

			float buttonMaxX = window->Pos.x + ImGui::CalcTextSize(applyText).x + ImGui::CalcTextSize(cancelText).x + style.FramePadding.x * 4 + style.ItemSpacing.x / 2;
			if (cursorMaxPos < buttonMaxX) {
				cursorMaxPos = buttonMaxX;
			}

			window->DC.CursorMaxPos.x = cursorMaxPos;

			ImGui::EndPopup();
		}

		return res;
	}
} // namespace ImGuiEx
// #endif
