#pragma once

#include <cstdint>
#include <string>

#include "arcdps_structs_slim.h"

enum class MountIndex : uint8_t {
	None,
	Jackal,
	Griffon,
	Springer,
	Skimmer,
	Raptor,
	Roller_Beetle,
	Warclaw,
	Skyscale,
	Skiff,
	SiegeTurtle
};

enum UiStateFlags_ : uint32_t {
	UiStateFlags_None = 0,
	UiStateFlags_MapOpen = 1 << 0,
	UiStateFlags_CompassTopRight = 1 << 1,
	UiStateFlags_CompassRotation = 1 << 2,
	UiStateFlags_GameFocus = 1 << 3,
	UiStateFlags_Competitive = 1 << 4,
	UiStateFlags_TextBoxFocus = 1 << 5,
	UiStateFlags_InCombat = 1 << 6,
};

typedef uint32_t UiStateFlags; // -> enum UiStateFlags_

// size: 88
struct MumbleContext {
	unsigned char serverAddress[28]; // contains sockaddr_in or sockaddr_in6
	uint32_t mapId;
	uint32_t mapType;
	uint32_t shardId;
	uint32_t instance;
	uint32_t buildId;
	// Additional data beyond the 48 bytes Mumble uses for identification
	UiStateFlags uiState;
	// Bitmask: Bit 1 = IsMapOpen, Bit 2 = IsCompassTopRight, Bit 3 = DoesCompassHaveRotationEnabled, Bit 4 = Game has focus, Bit 5 = Is in Competitive game mode, Bit 6 = Textbox has focus, Bit 7 = Is in Combat
	uint16_t compassWidth;  // pixels
	uint16_t compassHeight; // pixels
	float compassRotation;  // radians
	float playerX;          // continentCoords
	float playerY;          // continentCoords
	float mapCenterX;       // continentCoords
	float mapCenterY;       // continentCoords
	float mapScale;
	uint32_t processId;
	MountIndex mountIndex;
};

struct LinkedMem {
	uint32_t uiVersion;
	uint32_t uiTick;
	float fAvatarPosition[3];
	float fAvatarFront[3];
	float fAvatarTop[3];
	wchar_t name[256];
	float fCameraPosition[3];
	float fCameraFront[3];
	float fCameraTop[3];
	wchar_t identity[256]; // json of Identity struct
	uint32_t context_len;  // Despite the actual context containing more data, this value is always 48. See "context" section below.
	unsigned char context[256];
	wchar_t description[2048];

	MumbleContext* getMumbleContext() {
		return reinterpret_cast<MumbleContext*>(context);
	}
};

enum class Race : uint8_t {
	Asura,
	Charr,
	Human,
	Norn,
	Sylvari,
};

enum class UIScaling {
	Small,
	Normal,
	Large,
	Larger,
};

struct Identity {
	std::string name;       // Character name
	Prof profession;        // Character's profession
	uint32_t spec;          // Character's third specialization, or 0 if no specialization is present. See /v2/specializations for valid IDs.
	Race race;              // Character's race
	uint32_t mapId;         // Per API:2/maps
	uint32_t worldId;       // Formerly per API:2/worlds; not usable since the switch to the megaserver system
	uint32_t team_color_id; // Team color per API:2/colors (0 = white)
	bool commander;         // Whether the character has a commander tag active
	float fov;              // Vertical field-of-view
	UIScaling uisz;         // A value corresponding to the user's current UI scaling.
};
