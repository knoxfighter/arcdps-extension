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

const std::string& ArcdpsExtension::Localization::Translate(size_t pId) const {
	return mCurrentTranslation->at(pId);
}

void ArcdpsExtension::Localization::ChangeLanguage(gwlanguage pLang) {
	mCurrentLanguage = pLang;
	mCurrentTranslation = &mTranslations.at(pLang);
}

void ArcdpsExtension::Localization::SChangeLanguage(gwlanguage pLang) {
	Localization::instance().ChangeLanguage(pLang);
}
