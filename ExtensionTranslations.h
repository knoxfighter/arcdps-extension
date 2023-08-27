#pragma once

#include <array>

namespace ArcdpsExtension {
	enum ExtensionTranslation {
		ET_Left,
		ET_Right,
		ET_Center,
		ET_Unaligned,
		ET_PositionManual,
		ET_PositionScreenRelative,
		ET_PositionWindowRelative,
		ET_Unknown,
		ET_CornerPositionTopLeft,
		ET_CornerPositionTopRight,
		ET_CornerPositionBottomLeft,
		ET_CornerPositionBottomRight,
		ET_SizingPolicySizeToContent,
		ET_SizingPolicySizeContentToWindow,
		ET_SizingPolicyManualWindowSize,
		ET_KeyInputPopupName,
		ET_ApplyButton,
		ET_CancelButton,
		ET_UpdateDesc,
		ET_UpdateCurrentVersion,
		ET_UpdateNewVersion,
		ET_UpdateOpenPage,
		ET_UpdateAutoButton,
		ET_UpdateInProgress,
		ET_UpdateRestartPending,
		ET_UpdateError,
		ET_Style,
		ET_TitleBar,
		ET_Background,
		ET_Scrollbar,
		ET_Padding,
		ET_SizingPolicy,
		ET_AppearAsInOption,
		ET_TitleBarText,
		ET_Shortcut,
		ET_Position,
		ET_FromAnchorPanelCorner,
		ET_ThisPanelCorner,
		ET_AnchorWindow,
		ET_ColumnSetup,
		ET_ShowBasedOnMap,
		ET_AlternatingRowBg,
		ET_HighlightHoveredRow,
		ET_MaxDisplayed,
		ET_HeaderAlignment,
		ET_ColumnAlignment,
		ET_Language,
		ET_SettingsShowHeaderText,
	};

	constexpr auto EXTENSION_TRANSLATION_ENGLISH = std::to_array({
			"Left",                                                   // ET_Left,
			"Right",                                                  //     ET_Right,
			"Centered",                                               //     ET_Center,
			"Standard",                                               //     ET_Unaligned,
			"Manual",                                                 //     ET_PositionManual,
			"Screen Relative",                                        //     ET_PositionScreenRelative,
			"Window Relative",                                        // 	ET_PositionWindowRelative,
			"Unknown",                                                //     ET_Unknown,
			"Top-Left",                                               //     ET_CornerPositionTopLeft,
			"Top-Right",                                              //     ET_CornerPositionTopRight,
			"Bottom-Left",                                            //     ET_CornerPositionBottomLeft,
			"Bottom-Right",                                           //     ET_CornerPositionBottomRight,
			"Size to Content",                                        //     ET_SizingPolicySizeToContent,
			"Size Content to Window",                                 //     ET_SizingPolicySizeContentToWindow,
			"Manual Window Size",                                     //     ET_SizingPolicyManualWindowSize,
			"KeyBind",                                                //     ET_KeyInputPopupName,
			"Apply",                                                  //     ET_ApplyButton,
			"Cancel",                                                 //     ET_CancelButton,
			"A new update for the {} is available.",                  //     ET_UpdateDesc,
			"Current version",                                        //     ET_UpdateCurrentVersion,
			"New Version",                                            //     ET_UpdateNewVersion,
			"Open download page",                                     //     ET_UpdateOpenPage,
			"Update automatically",                                   //     ET_UpdateAutoButton,
			"Autoupdate in progress",                                 //     ET_UpdateInProgress,
			"Autoupdate finished, restart your game to activate it.", //     ET_UpdateRestartPending,
			"Autoupdate error, please update manually.",              //     ET_UpdateError,
			"Style",                                                  // ET_Style
			"Title bar",                                              // ET_TitleBar
			"Background",                                             // ET_Background
			"Scrollbar",                                              // ET_Scrollbar
			"Padding",                                                // ET_Padding
			"Sizing Policy",                                          // ET_SizingPolicy
			"Appear as in option",                                    // ET_AppearAsInOption
			"Title bar",                                              // ET_TitleBarText
			"Shortcut",                                               // ET_Shortcut
			"Position",                                               // ET_Position
			"From anchor panel corner",                               // ET_FromAnchorPanelCorner
			"This panel corner",                                      // ET_ThisPanelCorner
			"Anchor window",                                          // ET_AnchorWindow
			"Column Setup",                                           // ET_ColumnSetup
			"Show Columns based on map",                              // ET_ShowBasedOnMap
			"Alternating Row Background",                             // ET_AlternatingRowBg
			"Highlight hovered row",                                  // ET_HighlightHoveredRow
			"max displayed",                                          // ET_MaxDisplayed
			"Header Alignment",                                       // ET_HeaderAlignment
			"Column Alignment",                                       // ET_ColumnAlignment
			"Language",                                               // ET_Language
			"Show header with text instead of images",                // ET_SettingsShowHeaderText
	});

	const auto EXTENSION_TRANSLATION_GERMAN = std::to_array({
			"Links",                                                        // ET_Left,
			"Rechts",                                                       // ET_Right,
			"Zentriert",                                                    // ET_Center,
			"Standard",                                                     // ET_Unaligned,
			"Manuell",                                                      // ET_PositionManual,
			"Relativ zum Bildschirm",                                       // ET_PositionScreenRelative,
			"Relativ zu einem anderen Fenster",                             // ET_PositionWindowRelative,
			"Unbekannt",                                                    // ET_Unknown,
			"Oben-Links",                                                   // ET_CornerPositionTopLeft,
			"Oben-Rechts",                                                  // ET_CornerPositionTopRight,
			"Unten-Links",                                                  // ET_CornerPositionBottomLeft,
			"Unten-Rechts",                                                 // ET_CornerPositionBottomRight,
			"Passe Fenster an Inhalt an",                                   // ET_SizingPolicySizeToContent,
			"Passe Inhalt an Fenster an",                                   // ET_SizingPolicySizeContentToWindow,
			(const char*) u8"Manuelle Fenstergröße",                        // ET_SizingPolicyManualWindowSize,
			"Tastenbelegung",                                               // ET_KeyInputPopupName,
			"Anwenden",                                                     // ET_ApplyButton,
			"Abbrechen",                                                    // ET_CancelButton,
			"Eine neue Version für das {} ist verfügbar.",                  // ET_UpdateDesc,
			"Aktuelle Version",                                             // ET_UpdateCurrentVersion,
			"Neue Version",                                                 // ET_UpdateNewVersion,
			(const char*) u8"Öffne Download Seite",                         // ET_UpdateOpenPage,
			"Automatisch Aktualisieren",                                    // ET_UpdateAutoButton,
			"Aktualisierung im Gange",                                      // ET_UpdateInProgress,
			"Aktualisierung beendet, starte das Spiel neu zum Aktivieren.", // ET_UpdateRestartPending,
			"Aktualisierung fehlgeschlagen, bitte update manuell.",         // ET_UpdateError,
			"Style",                                                        // ET_Style
			"Titelleiste",                                                  // ET_TitleBar
			"Hintergrund",                                                  // ET_Background
			"Scrollleiste",                                                 // ET_Scrollbar
			"Padding",                                                      // ET_Padding
			(const char*) u8"Größenregeln",                                 // ET_SizingPolicy
			"Optionstext",                                                  // ET_AppearAsInOption
			"Titelleiste Text",                                             // ET_TitleBarText
			(const char*) u8"Tastenkürzel",                                 // ET_Shortcut
			"Position",                                                     // ET_Position
			"Ecke des anzuheftenden Fensters",                              // ET_FromAnchorPanelCorner
			"Ecke des aktuellen Fensters",                                  // ET_ThisPanelCorner
			"Anzuheftendes Fenster",                                        // ET_AnchorWindow
			"Spalteneinstellung",                                           // ET_ColumnSetup
			"Zeige Spalten basierend auf der aktuellen Karte.",             // ET_ShowBasedOnMap
			"Abwechselnder Zeilenhintergrund",                              // ET_AlternatingRowBg
			"Markiere die aktuelle Zeile",                                  // ET_HighlightHoveredRow
			"Maximale Anzahl an Zeilen",                                    // ET_MaxDisplayed
			"Ausrichtung der Kopfzeile",                                    // ET_HeaderAlignment
			"Ausrichtung des Inhalts",                                      // ET_ColumnAlignment
			"Sprache",                                                      // ET_Language
			"Zeige Text anstatt von Icons in der Kopfzeile",                // ET_SettingsShowHeaderText
	});

	constexpr auto EXTENSION_TRANSLATION_FRENCH = std::to_array({
			u8"Gauche",                                                                        // ET_Left,
			u8"Droit",                                                                         //     ET_Right,
			u8"Centré",                                                                        //     ET_Center,
			u8"Standard",                                                                      //     ET_Unaligned,
			u8"Manuel",                                                                        //     ET_PositionManual,
			u8"Écran Relatif",                                                                 //     ET_PositionScreenRelative,
			u8"Fenêtre relative",                                                              //  ET_PositionWindowRelative,
			u8"Inconnu",                                                                       //     ET_Unknown,
			u8"Coin supérieur gauche",                                                         //     ET_CornerPositionTopLeft,
			u8"Coin supérieur droit",                                                          //     ET_CornerPositionTopRight,
			u8"Coin inférieur gauche",                                                         //     ET_CornerPositionBottomLeft,
			u8"Coin inférieur droit",                                                          //     ET_CornerPositionBottomRight,
			u8"Taille par rapport au contenu",                                                 //     ET_SizingPolicySizeToContent,
			u8"Taille du contenu par rapport à la fenêtre",                                    //     ET_SizingPolicySizeContentToWindow,
			u8"Taille de la fenêtre manuelle",                                                 //     ET_SizingPolicyManualWindowSize,
			u8"Lier les touches",                                                              //     ET_KeyInputPopupName,
			u8"Appliquer",                                                                     //     ET_ApplyButton,
			u8"Annuler",                                                                       //     ET_CancelButton,
			u8"Une nouvelle mise à jour pour le {} est disponible.",                           //     ET_UpdateDesc,
			u8"Version actuelle",                                                              //     ET_UpdateCurrentVersion,
			u8"Nouvelle version",                                                              //     ET_UpdateNewVersion,
			u8"Ouvrir la page de téléchargement",                                              //     ET_UpdateOpenPage,
			u8"Mise à jour automatique",                                                       //     ET_UpdateAutoButton,
			u8"Mise à jour automatique en cours",                                              //     ET_UpdateInProgress,
			u8"La mise à jour automatique est terminée, redémarrez votre jeu pour l'activer.", //     ET_UpdateRestartPending,
			u8"Erreur de mise à jour automatique, veuillez mettre à jour manuellement.",       //     ET_UpdateError,
			u8"Style",                                                                         // ET_Style
			u8"Barre de titre",                                                                // ET_TitleBar
			u8"Fond d'écran",                                                                  // ET_Background
			u8"Barre de défilement",                                                           // ET_Scrollbar
			u8"Padding",                                                                       // ET_Padding
			u8"Règle de dimensionnement",                                                      // ET_SizingPolicy
			u8"Apparaître comme dans l'option",                                                // ET_AppearAsInOption
			u8"Barre de titre",                                                                // ET_TitleBarText
			u8"Raccourci",                                                                     // ET_Shortcut
			u8"Position",                                                                      // ET_Position
			u8"Depuis le coin du panneau d'ancrage",                                           // ET_FromAnchorPanelCorner
			u8"Ce coin de panneau",                                                            // ET_ThisPanelCorner
			u8"Fenêtre d'ancrage",                                                             // ET_AnchorWindow
			u8"Configuration des colonnes",                                                    // ET_ColumnSetup
			u8"Afficher les colonnes en fonction de la carte",                                 // ET_ShowBasedOnMap
			u8"Fond à rangs alternés",                                                         // ET_AlternatingRowBg
			u8"Mettre en surbrillance la ligne survolée",                                      // ET_HighlightHoveredRow
			u8"max affiché",                                                                   // ET_MaxDisplayed
			u8"Alignement de l'en-tête",                                                       // ET_HeaderAlignment
			u8"Alignement des colonnes",                                                       // ET_ColumnAlignment
			u8"Langue",                                                                        // ET_Language
			u8"Afficher l'en-tête avec du texte au lieu des images",                           // ET_SettingsShowHeaderText
	});

	constexpr auto EXTENSION_TRANSLATION_SPANISH = std::to_array({
			u8"Izquierda",                                                                    // ET_Left,
			u8"Derecha",                                                                      //     ET_Right,
			u8"Centrado",                                                                     //     ET_Center,
			u8"Estándar",                                                                     //     ET_Unaligned,
			u8"Manual",                                                                       //     ET_PositionManual,
			u8"Relativo a la pantalla",                                                       //     ET_PositionScreenRelative,
			u8"Relativo a la ventana",                                                        // 	ET_PositionWindowRelative,
			u8"Desconocido",                                                                  //     ET_Unknown,
			u8"Arriba a la izquierda",                                                        //     ET_CornerPositionTopLeft,
			u8"Arriba a la derecha",                                                          //     ET_CornerPositionTopRight,
			u8"Abajo a la izquierda",                                                         //     ET_CornerPositionBottomLeft,
			u8"Abajo a la derecha",                                                           //     ET_CornerPositionBottomRight,
			u8"Tamaño al contenido",                                                          //     ET_SizingPolicySizeToContent,
			u8"Tamaño del contenido a la ventana",                                            //     ET_SizingPolicySizeContentToWindow,
			u8"Tamaño de la ventana de Manuel",                                               //     ET_SizingPolicyManualWindowSize,
			u8"Encuadernación de teclas",                                                     //     ET_KeyInputPopupName,
			u8"Aplicar",                                                                      //     ET_ApplyButton,
			u8"Cancelar",                                                                     //     ET_CancelButton,
			u8"Está disponible una nueva actualización para el {}.",                          //     ET_UpdateDesc,
			u8"Versión actual",                                                               //     ET_UpdateCurrentVersion,
			u8"Nueva versión",                                                                //     ET_UpdateNewVersion,
			u8"Abrir la página de descarga",                                                  //     ET_UpdateOpenPage,
			u8"Actualizar automáticamente",                                                   //     ET_UpdateAutoButton,
			u8"Actualización automática en curso",                                            //     ET_UpdateInProgress,
			u8"La actualización automática ha finalizado, reinicia el juego para activarla.", //     ET_UpdateRestartPending,
			u8"Error de actualización automática, por favor, actualice manualmente.",         //     ET_UpdateError,
			u8"Estilo",                                                                       // ET_Style
			u8"Barra de título",                                                              // ET_TitleBar
			u8"Antecedentes",                                                                 // ET_Background
			u8"Barra de desplazamiento",                                                      // ET_Scrollbar
			u8"Padding",                                                                      // ET_Padding
			u8"Política de tallas",                                                           // ET_SizingPolicy
			u8"Aparecer como en la opción",                                                   // ET_AppearAsInOption
			u8"Barra de título",                                                              // ET_TitleBarText
			u8"Atajo",                                                                        // ET_Shortcut
			u8"Posición",                                                                     // ET_Position
			u8"Desde la esquina del panel de anclaje",                                        // ET_FromAnchorPanelCorner
			u8"Esta esquina del panel",                                                       // ET_ThisPanelCorner
			u8"Ventana de anclaje",                                                           // ET_AnchorWindow
			u8"Configuración de la columna",                                                  // ET_ColumnSetup
			u8"Mostrar columnas basadas en el mapa",                                          // ET_ShowBasedOnMap
			u8"Fondo de filas alternas",                                                      // ET_AlternatingRowBg
			u8"Resaltar la fila que se ha desplazado",                                        // ET_HighlightHoveredRow
			u8"máximo mostrado",                                                              // ET_MaxDisplayed
			u8"Alineación de la cabecera",                                                    // ET_HeaderAlignment
			u8"Alineación de columnas",                                                       // ET_ColumnAlignment
			u8"Idioma",                                                                       // ET_Language
			u8"Mostrar la cabecera con texto en lugar de imágenes",                           // ET_SettingsShowHeaderText
	});
} // namespace ArcdpsExtension

#if __has_include("magic_enum.hpp")
#include <magic_enum.hpp>
static_assert(ArcdpsExtension::EXTENSION_TRANSLATION_ENGLISH.size() == magic_enum::enum_count<ArcdpsExtension::ExtensionTranslation>());
#endif
static_assert(ArcdpsExtension::EXTENSION_TRANSLATION_ENGLISH.size() == ArcdpsExtension::EXTENSION_TRANSLATION_GERMAN.size());
static_assert(ArcdpsExtension::EXTENSION_TRANSLATION_ENGLISH.size() == ArcdpsExtension::EXTENSION_TRANSLATION_FRENCH.size());
static_assert(ArcdpsExtension::EXTENSION_TRANSLATION_ENGLISH.size() == ArcdpsExtension::EXTENSION_TRANSLATION_SPANISH.size());
