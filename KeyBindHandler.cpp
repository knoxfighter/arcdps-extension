#include "KeyBindHandler.h"

#include "arcdps_structs.h"

#include <format>
#include <ranges>

uint64_t ArcdpsExtension::KeyBindHandler::Subscribe(Subscriber pSubscriber) {
	uint64_t newId = getNewId();
	mSubscribers.try_emplace(newId, pSubscriber);
	return newId;
}

void ArcdpsExtension::KeyBindHandler::Unsubscribe(uint64_t pId) {
	mSubscribers.erase(pId);
}

void ArcdpsExtension::KeyBindHandler::UpdateKey(uint64_t pId, const KeyBinds::Key& pKey) {
	mSubscribers.at(pId).Key = pKey;
}

void ArcdpsExtension::KeyBindHandler::UpdateKeys(const KeyBinds::Key& pOldKey, const KeyBinds::Key& pNewKey) {
	for (auto& subscriber : mSubscribers | std::views::values) {
		if (subscriber.Key == pOldKey) {
			subscriber.Key = pNewKey;
		}
	}
}

bool ArcdpsExtension::KeyBindHandler::Wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN: {
			UINT scanCode = (lParam >> 16) & 0x00ff;
			const bool extended = (lParam >> 24) & 0x1;
			const bool previous = (lParam >> 30) & 0x1;

			if (previous == true) break;

			if (extended == true) {
				scanCode |= 0xE000;
			}

			const auto& keyCodeOpt = KeyBinds::MsvcScanCodeToKeyCode(scanCode);
			if (!keyCodeOpt) {
				ARC_LOG(std::format("unknown key: {}", scanCode).c_str());
				break;
			}
			const KeyBinds::KeyCode& keyCode = keyCodeOpt.value();
			KeyBinds::Modifier modifier = KeyBinds::GetModifier(keyCode);
			if (modifier != 0) {
				mTrackedModifier |= modifier;
			} else {
				KeyBinds::Modifier arcdpsModifier = GetArcdpsModifier();

				// Check if any registered KeyBind is correct
				for (const auto& subscriber : mSubscribers | std::views::values) {
					const auto& key = subscriber.Key;

					if ((subscriber.Flags & SubscriberFlags_ArcdpsModifier ? arcdpsModifier == mTrackedModifier : key.Modifier == mTrackedModifier) && key.DeviceType == KeyBinds::DeviceType::Keyboard && static_cast<KeyBinds::KeyCode>(key.Code) == keyCode) {
						bool res = subscriber.Fun(key);
						if (res) {
							return res;
						}
					}
				}
			}
			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP: {
			UINT scanCode = (lParam >> 16) & 0x00ff;
			const bool extended = (lParam >> 24) & 0x1;

			if (extended == true) {
				scanCode |= 0xE000;
			}

			const auto& keyCode = KeyBinds::MsvcScanCodeToKeyCode(scanCode);
			if (!keyCode) break;
			KeyBinds::Modifier modifier = KeyBinds::GetModifier(keyCode.value());
			if (modifier != 0) {
				mTrackedModifier &= !modifier;
			}
			break;
		}
		case WM_XBUTTONDOWN: {
			int32_t code = 0;
			WORD word = GET_XBUTTON_WPARAM(wParam);
			if (word == XBUTTON1) {
				code = static_cast<int32_t>(KeyBinds::MouseCode::Mouse_4);
			} else if (word == XBUTTON2) {
				code = static_cast<int32_t>(KeyBinds::MouseCode::Mouse_5);
			}

			if (code != 0) {
				for (const auto& subscriber : mSubscribers) {
					const auto& key = subscriber.second.Key;
					if (key.Modifier == mTrackedModifier && key.DeviceType == KeyBinds::DeviceType::Mouse && key.Code == code) {
						return subscriber.second.Fun(key);
					}
				}
			}
			break;
		}
		case WM_MBUTTONDOWN: {
			for (const auto& subscriber : mSubscribers) {
				const auto& key = subscriber.second.Key;
				if (key.Modifier == mTrackedModifier && key.DeviceType == KeyBinds::DeviceType::Mouse && key.Code == static_cast<int32_t>(KeyBinds::MouseCode::Mouse_3)) {
					return subscriber.second.Fun(key);
				}
			}
			break;
		}
		case WM_ACTIVATEAPP: {
			// reset modifier when loosing focus (Alt+Tab)
			mTrackedModifier = 0;

			break;
		}
	}

	return false;
}

KeyBinds::Modifier ArcdpsExtension::KeyBindHandler::GetArcdpsModifier() {
	uint64_t e7_result = ARC_EXPORT_E7();

	uint16_t* ra = (uint16_t*) &e7_result;
	if (ra) {
		uint16_t mod1 = ra[0];
		uint16_t mod2 = ra[1];

		return getArcdpsModifierSingle(mod1) | getArcdpsModifierSingle(mod2);
	}

	return 0;
}

KeyBinds::Modifier ArcdpsExtension::KeyBindHandler::getArcdpsModifierSingle(uint16_t pMod) {
	if (pMod == 16) {
		return KeyBinds::Modifier_Shift;
	}
	if (pMod == 18) {
		return KeyBinds::Modifier_Alt;
	}
	if (pMod == 17) {
		return KeyBinds::Modifier_Ctrl;
	}
	return 0;
}
