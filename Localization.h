#pragma once

#include "arcdps_structs_slim.h"
#include "Singleton.h"

#include <array>
#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

namespace ArcdpsExtension {
	/**
	 * How to use:<br>
	 * Call `Load()` with your translations you want to add.
	 * Things are added to the back of the list with all `Extension` based translations at the beginning.
	 * So make sure, your IDs enum or whatever you use, starts with one higher than the last used one.
	 * Also make sure you add all 4 translations and they are of the same size.
	 * Call it within `mod_init` or as early as possible.
	 *
	 * Call `ChangeLanguage` when you want to change the language.
	 * Call it once to setup the language the user has in `mod_init`
	 */
	class Localization final : public Singleton<Localization> {
	public:
		Localization();

		[[nodiscard]] const std::string& Translate(size_t pId) const;

		template<typename E>
		requires std::is_enum_v<E>
		[[nodiscard]] const std::string& Translate(E pId) const {
			return Translate(static_cast<std::underlying_type_t<E>>(pId));
		}

		template<typename Num>
		requires std::is_integral_v<Num> && (!std::same_as<Num, size_t>)
		[[nodiscard]] const std::string& Translate(Num pId) const {
			return Translate(static_cast<size_t>(pId));
		}

		template<typename T>
		[[nodiscard]] static const std::string& STranslate(T pId) {
			return Localization::instance().Translate(pId);
		}

		void AddTranslation(gwlanguage pLang, const char8_t* pText) {
			AddTranslation(pLang, reinterpret_cast<const char*>(pText));
		}

		template<typename... Args>
		void AddTranslation(gwlanguage pLang, Args&&... args) {
			auto& translation = mTranslations.at(pLang);
			translation.emplace_back(std::forward<Args>(args)...);
		}

		void Load(gwlanguage pLang, const std::ranges::common_range auto& pRange) {
			for (const auto& value : pRange) {
				AddTranslation(pLang, value);
			}
		}

		void ChangeLanguage(gwlanguage pLang);
		static void SChangeLanguage(gwlanguage pLang);

		template<typename... Args>
		void OverrideTranslation(gwlanguage pLanguage, size_t pTranslation, Args&&... args) {
			mTranslations.at(pLanguage).at(pTranslation) = std::move(std::string(std::forward<Args...>(args...)));
		}

		template<typename E, typename... Args>
		requires std::is_enum_v<E>
		void OverrideTranslation(gwlanguage pLanguage, E pTranslation, Args&&... args) {
			OverrideTranslation(pLanguage, static_cast<std::underlying_type_t<E>>(pTranslation), std::forward<Args...>(args...));
		}

		template<typename Num, typename... Args>
		requires std::is_integral_v<Num> && (!std::same_as<Num, size_t>)
		void OverrideTranslation(gwlanguage pLanguage, Num pTranslation, Args&&... args) {
			OverrideTranslation(pLanguage, static_cast<size_t>(pTranslation), std::forward<Args...>(args...));
		}

	private:
		std::array<std::vector<std::string>, 6> mTranslations;
		gwlanguage mCurrentLanguage = GWL_ENG;
		std::vector<std::string>* mCurrentTranslation;
	};
} // namespace ArcdpsExtension
