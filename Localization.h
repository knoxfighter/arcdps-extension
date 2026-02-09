#pragma once

#include "arcdps_structs_slim.h"
#include "Singleton.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace ArcdpsExtension {
	/**
	 * How to use:<br>
	 * Call `Load()` with your translations you want to add.
	 * Things are added to the back of the list with all `Extension` based translations at the beginning.
	 * So make sure, your IDs enum or whatever you use, starts with one higher than the last used one.
	 * Also make sure you add all 5 translations and they are of the same size.
	 * Call it within `mod_init` or as early as possible.
	 *
	 * Call `ChangeLanguage` when you want to change the language.
	 * Call it once to setup the language the user has in `mod_init`
	 */
	class Localization final : public Singleton<Localization> {
	public:
		Localization();

		[[nodiscard]] std::string_view Translate(size_t pId) const;
		[[nodiscard]] std::string_view Translate(auto lang, size_t pId) const {
			return mTranslations.at(static_cast<size_t>(lang)).at(pId);
		}

		template<typename E>
		requires std::is_enum_v<E>
		[[nodiscard]] std::string_view Translate(E pId) const {
			return Translate(std::to_underlying(pId));
		}
		template<typename E>
		requires std::is_enum_v<E>
		[[nodiscard]] std::string_view Translate(auto lang, E pId) const {
			return Translate(lang, std::to_underlying(pId));
		}

		template<typename Num>
		requires std::is_integral_v<Num> && (!std::same_as<Num, size_t>)
		[[nodiscard]] std::string_view Translate(Num pId) const {
			return Translate(static_cast<size_t>(pId));
		}
		template<typename Num>
		requires std::is_integral_v<Num> && (!std::same_as<Num, size_t>)
		[[nodiscard]] std::string_view Translate(auto lang, Num pId) const {
			return Translate(lang, static_cast<size_t>(pId));
		}

		/**
		 *
		 * @return null terminated string_view. Use .data() instead of c_str() if you need a const char*
		 */
		template<typename... Args>
		[[nodiscard]] static std::string_view STranslate(Args... args) {
			return Localization::instance().Translate(std::forward<Args>(args)...);
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

	// Same as arcdps' gwlanguage, but contains LikeGame option instead of KR.
	// Use this in your settings object and handle LikeGame manually.
	enum class LanguageSetting {
		English = 0,
		LikeGame = 1,
		French = 2,
		German = 3,
		Spanish = 4,
		Chinese = 5,
	};
} // namespace ArcdpsExtension

std::string_view to_string(ArcdpsExtension::LanguageSetting pLang);
