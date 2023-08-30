#pragma once

#include "Singleton.h"

#include <ArcdpsUnofficialExtras/KeyBindHelper.h>
#include <functional>
#include <Windows.h>

namespace ArcdpsExtension {
	/**
	 * To Use:<br>
	 * - This class only works with arcdps<br>
	 * - Make sure you have set tha extern variables in `arcdps_structs.h`. Those are called here.<br>
	 * - The Singleton also has to be setup<br>
	 * - Call `Wnd` in `mod_wnd` (arcdps callback)<br>
	 */
	class KeyBindHandler : public Singleton<KeyBindHandler> {
	public:
		enum SubscriberFlags_ {
			SubscriberFlags_None = 0,
			SubscriberFlags_ArcdpsModifier = 1 << 0,
		};
		using SubscriberFlags = std::underlying_type_t<SubscriberFlags_>; // enum SubscriberFlags_

		/**
		 * The function that is called when a Key-combination is pressed.
		 * \return boolean if the event should be consumed.
		 */
		using SubscriberFun = std::function<bool(const KeyBinds::Key&)>;

		struct Subscriber {
			KeyBinds::Key Key;
			SubscriberFun Fun;
			SubscriberFlags Flags;
		};

		/**
	 	 * Subscribe a function to a Key that is called when the Key-combination is pressed.
		 *
		 * \return ID of the Subscription, use this in Unsubscribe.
		 */
		uint64_t Subscribe(Subscriber pSubscriber);

		/**
		 * Unsubscribe given Subscription ID
		 */
		void Unsubscribe(uint64_t pId);

		/**
		 * Update the Key of the given Subscription ID
		 */
		void UpdateKey(uint64_t pId, const KeyBinds::Key& pKey);

		/**
		 * Update all existing `pOldKey` tracked values to `pNewKey`
		 */
		void UpdateKeys(const KeyBinds::Key& pOldKey, const KeyBinds::Key& pNewKey);

		/**
		 * Call this in `mod_wnd`.
		 */
		bool Wnd(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		/**
		 * Call to get the arcdps modifier as KeyBindModifier (here cause it cannot be in KeyBindHelper)
		 */
		static KeyBinds::Modifier GetArcdpsModifier();

	private:
		static uint64_t getNewId() {
			static uint64_t the_id;
			return the_id++;
		}

		std::unordered_map<uint64_t, Subscriber> mSubscribers;
		KeyBinds::Modifier mTrackedModifier = 0;

		static KeyBinds::Modifier getArcdpsModifierSingle(uint16_t pMod);
	};
} // namespace ArcdpsExtension
