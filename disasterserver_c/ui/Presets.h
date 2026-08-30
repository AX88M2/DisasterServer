#ifndef PRESETS_H
#define PRESETS_H
#include <Maps.h>
#include <stdbool.h>

typedef struct
{
	const char* name;
	const bool values[MAP_COUNT];
} Preset;

extern Preset g_defaultPresets[];
#define PRESET_COUNT 9
#define PRESET_CUSTOM 8

extern int g_defaultPingPresets[];
#define PING_PRESET_COUNT 5

#endif