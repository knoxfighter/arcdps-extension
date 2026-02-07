#include "Localization.h"

#include "ExtensionTranslations.h"

ArcdpsExtension::Localization::Localization() {
	mCurrentTranslation = &mTranslations.at(mCurrentLanguage);

	Load(GWL_ENG, EXTENSION_TRANSLATION_ENGLISH);
	Load(GWL_GEM, EXTENSION_TRANSLATION_GERMAN);
	Load(GWL_SPA, EXTENSION_TRANSLATION_SPANISH);
	Load(GWL_FRE, EXTENSION_TRANSLATION_FRENCH);
	Load(GWL_CN, EXTENSION_TRANSLATION_CHINESE);
}

std::string_view ArcdpsExtension::Localization::Translate(size_t pId) const {
	return mCurrentTranslation->at(pId);
}

void ArcdpsExtension::Localization::ChangeLanguage(gwlanguage pLang) {
	mCurrentLanguage = pLang;
	mCurrentTranslation = &mTranslations.at(pLang);
}

void ArcdpsExtension::Localization::SChangeLanguage(gwlanguage pLang) {
	Localization::instance().ChangeLanguage(pLang);
}

std::string_view to_string(ArcdpsExtension::LanguageSetting pLang) {
	switch (pLang) {
		case ArcdpsExtension::LanguageSetting::English:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::LanguageSetting::English, ArcdpsExtension::ET_LanguageName);
		case ArcdpsExtension::LanguageSetting::LikeGame:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::ET_LikeInGame);
		case ArcdpsExtension::LanguageSetting::French:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::LanguageSetting::French, ArcdpsExtension::ET_LanguageName);
		case ArcdpsExtension::LanguageSetting::German:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::LanguageSetting::German, ArcdpsExtension::ET_LanguageName);
		case ArcdpsExtension::LanguageSetting::Spanish:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::LanguageSetting::Spanish, ArcdpsExtension::ET_LanguageName);
		case ArcdpsExtension::LanguageSetting::Chinese:
			return ArcdpsExtension::Localization::STranslate(ArcdpsExtension::LanguageSetting::Chinese, ArcdpsExtension::ET_LanguageName);
		default:
			return "Error, if you see this, please report it to the developer";
	}
}
