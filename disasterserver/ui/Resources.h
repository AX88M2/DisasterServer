#ifndef RESOURCES_H
#define RESOURCES_H
#include <Server.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

#define INFO_TEXT CLRCODE_PUR "disasterserverui " CLRCODE_BLU __DATE__  " " CLRCODE_GRN __TIME__ CLRCODE_RST "\n"\
CLRCODE_RST "(c) 2024 team exe empire\n\n\n"\
"game version: v" CLRCODE_PUR STRINGIFY(BUILD_VERSION) "~\n"\
"server version: " CLRCODE_YLW STRINGIFY(BUILD_MOD_VER) "\n\n"\
CLRCODE_RST "compiler: " CLRCODE_PUR "%s\n"\
CLRCODE_RST "sdl3 version: " CLRCODE_PUR "%d.%d.%d\n"\
CLRCODE_RST "sdl3_image version: " CLRCODE_PUR "%d.%d.%d\n\n"\
CLRCODE_RST "server icon by " CLRCODE_BLU "danik" "\n"\
CLRCODE_RST "developed by " CLRCODE_BLU "miles" CLRCODE_PUR "glitch" "\n"\
CLRCODE_RST "help with port by " CLRCODE_GRA "faker" CLRCODE_RED "null" CLRCODE_GRN "0" "\n"

extern SDL_Texture* g_textureSheet;
bool resources_load(SDL_Renderer* renderer);

#endif