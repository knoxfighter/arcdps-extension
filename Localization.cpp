#include "Localization.h"

#include "ExtensionTranslations.h"

Localization::Localization() {
	mCurrentTranslation = &mTranslations.at(mCurrentLanguage);

	Load(GWL_ENG, EXTENSION_TRANSLATION_ENGLISH);
	Load(GWL_GEM, EXTENSION_TRANSLATION_GERMAN);
	Load(GWL_SPA, EXTENSION_TRANSLATION_SPANISH);
	Load(GWL_FRE, EXTENSION_TRANSLATION_FRENCH);
}

const std::string& Localization::Translate(size_t pId) const {
	return mCurrentTranslation->at(pId);
}

void Localization::ChangeLanguage(gwlanguage pLang) {
	mCurrentLanguage = pLang;
	mCurrentTranslation = &mTranslations.at(pLang);
}

void Localization::SChangeLanguage(gwlanguage pLang) {
	Localization::instance().ChangeLanguage(pLang);
}

void Localization::OverrideTranslation(gwlanguage pLanguage, size_t pTranslation, const std::string& pText) {
	mTranslations.at(pLanguage).at(pTranslation) = pText;
}
