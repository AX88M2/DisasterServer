#include "Colors.h"
#include <ui/Components.h>
#include <ui/Presets.h>

bool ping_update(SDL_Renderer* renderer, struct _Component* component)
{
    PingLimit* list = (PingLimit*)component;
    SDL_Rect src = { 4272, 0, 128, 40 };
    SDL_Rect dst = { list->x * INTERFACE_SCALE, list->y * INTERFACE_SCALE, list->w * INTERFACE_SCALE, list->h * INTERFACE_SCALE };

    int mouse_x, mouse_y;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);

    bool mouse_down = false;
    if (flags & SDL_BUTTON(1))
    {
        if (!list->clicked)
        {
            mouse_down = true;
            list->clicked = true;
        }
    }
    else
        list->clicked = false;

    mouse_x /= scale_x;
    mouse_y /= scale_y;

    if (mouse_x >= dst.x && mouse_y >= dst.y && mouse_x < dst.x + dst.w && mouse_y < dst.y + dst.h)
    {
        src.x += src.w;

        SDL_Rect arrow_src = { 1680, 0, 116, 32 };
        SDL_Rect arrow_dst = { dst.x, dst.y, 116 * INTERFACE_SCALE, 32 * INTERFACE_SCALE };

        const char* text;

        if (mouse_down)
        {
            arrow_dst.y += 2;

            if (mouse_x < dst.x + dst.w / 2)
            {
                if (--list->preset < 0)
                    list->preset = PING_PRESET_COUNT - 1;
            }
            else
            {
                if (++list->preset >= PING_PRESET_COUNT)
                    list->preset = 0;
            }

            g_config.ping_limit = g_defaultPingPresets[list->preset];
            config_save();
        }

        SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);
        SDL_RenderCopy(renderer, g_textureSheet, &arrow_src, &arrow_dst);
    }
    else
        SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

    char* name;
    switch (g_config.ping_limit)
    {
    case 80:
        name = CLRCODE_RED "very strict";
        list->preset = 0;
        break;

    case 120:
        name = CLRCODE_YLW "strict";
        list->preset = 1;
        break;

    case 180:
        name = "forgiving";
        list->preset = 2;
        break;

    case 250:
        name = CLRCODE_GRN "defualt";
        list->preset = 3;
        break;

    case UINT16_MAX:
        name = CLRCODE_PUR "unlimited";
        list->preset = 4;
        break;

    default:
        name = "custom";
        list->preset = 0;
        break;
    }

    list->label.scale = INTERFACE_SCALE;
    list->label.x = dst.x / INTERFACE_SCALE + 8;
    list->label.y = dst.y / INTERFACE_SCALE + 25;

    char value[128];

    if(list->preset == 4)
        snprintf(value, 128, "%s", name);
    else
        snprintf(value, 128, "%s - " CLRCODE_RST "%d", name, g_config.ping_limit);
    
    list->label.text = value;
    label_update(renderer, (Component*)&list->label);
    return true;
}