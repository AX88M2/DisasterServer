#include <ui/Presets.h>

Preset g_defaultPresets[] = 
{
	{ "default",  { true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true } },
	{ "classic", { false, false, true, false, true, false, true, true, true, false, false, false, true, false, true, false, false, false, false, false } },
	{ "easy", { true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, true } },
	{ "medium", { false, true, false, true, true, false, false, true, false, false, false, true, true, false, true, false, false, false, false, false } },
	{ "hard", { false, false, false, false, false, true, true, false, false, true, false, false, false, false, false, true, false, false, true, false } },
	{ "madness", { false, false, false, false, false, false, false, false, true, false, true, false, false, true, false, false, false, false, false, false } },
	{ "nightmare universe", { false, true, false, true, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false } },
	{ "spirits of hell", { true, false, false, false, false, true, false, false, false, false, false, true, false, true, false, false, false, false, false, false } },
	{ "custom", { 0 } },
};

int g_defaultPingPresets[] = { 80, 120, 180, 250, UINT16_MAX };