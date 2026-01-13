#include "ExtensionTranslations.h"
#include "Localization.h"

#include <gtest/gtest.h>

using namespace ArcdpsExtension;

TEST(LocalizationTests, BaseTranslations) {
	Localization localization;

	localization.ChangeLanguage(GWL_ENG);
	ASSERT_EQ(localization.Translate(ET_Left), "Left");
	localization.ChangeLanguage(GWL_GEM);
	ASSERT_EQ(localization.Translate(ET_Left), "Links");
	localization.ChangeLanguage(GWL_FRE);
	ASSERT_EQ(localization.Translate(ET_Left), "Gauche");
	localization.ChangeLanguage(GWL_SPA);
	ASSERT_EQ(localization.Translate(ET_Left), "Izquierda");
	localization.ChangeLanguage(GWL_CN);
	ASSERT_EQ(localization.Translate(ET_Left), "居左");
}

TEST(LocalizationTests, BaseTranslationsSpecialChars) {
	Localization localization;

	localization.ChangeLanguage(GWL_GEM);
	ASSERT_EQ(localization.Translate(ET_SizingPolicyManualWindowSize), "Manuelle Fenstergröße");
	localization.ChangeLanguage(GWL_FRE);
	ASSERT_EQ(localization.Translate(ET_UpdateRestartPending), "La mise à jour automatique est terminée, redémarrez votre jeu pour l'activer.");
	localization.ChangeLanguage(GWL_SPA);
	ASSERT_EQ(localization.Translate(ET_UpdateInProgress), "Actualización automática en curso");
	localization.ChangeLanguage(GWL_CN);
	ASSERT_EQ(localization.Translate(ET_SizingPolicySizeContentToWindow), "根据窗口大小调整内容大小");
}

TEST(LocalizationTests, OverrideTranslation) {
	Localization localization;

	localization.ChangeLanguage(GWL_ENG);
	ASSERT_EQ(localization.Translate(ET_Left), "Left");
	localization.OverrideTranslation(GWL_ENG, ET_Left, "Rêchts");
	ASSERT_EQ(localization.Translate(ET_Left), "Rêchts");
}
