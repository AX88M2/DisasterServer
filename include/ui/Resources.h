#ifndef RESOURCES_H
#define RESOURCES_H
#include <Server.h>
#include <SDL2/SDL.h>
#include <stdbool.h>

#define INFO_TEXT CLRCODE_PUR "disasterserverui " CLRCODE_BLU __DATE__  " " CLRCODE_GRN __TIME__ CLRCODE_RST "\n"\
CLRCODE_RST "(c) 2024 team exe empire\n\n\n"\
"game version: v" CLRCODE_PUR STRINGIFY(BUILD_VERSION) "\n"\
CLRCODE_RST "compiler: " CLRCODE_PUR "%s\n"\
CLRCODE_RST "sdl2 version: " CLRCODE_PUR "%d.%d.%d\n"\
CLRCODE_RST "sdl2_image version: " CLRCODE_PUR "%d.%d.%d\n"\

extern SDL_Texture* g_textureSheet;
bool resources_load(SDL_Renderer* renderer);

#endif
