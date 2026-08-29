#include <ui/Components.h>
#include <ui/Presets.h>

bool mappreset_update(SDL_Renderer* renderer, struct _Component* component)
{
    MapListPreset* list = (MapListPreset*)component;
    SDL_Rect src = { 4016, 0, 128, 40 };
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

        if (mouse_down)
        {
            arrow_dst.y += 2;

            MutexLock(g_config.map_list_lock);
            {
                if (mouse_x < dst.x + dst.w / 2)
                {
                    if (--list->preset < 0)
                        list->preset = PRESET_COUNT - 2;
                }
                else
                {
                    if (++list->preset >= PRESET_COUNT - 1)
                        list->preset = 0;
                }

                memcpy(g_config.map_list, g_defaultPresets[list->preset].values, sizeof(g_config.map_list));
            }
            MutexUnlock(g_config.map_list_lock);
        }

        SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);
        SDL_RenderCopy(renderer, g_textureSheet, &arrow_src, &arrow_dst);
    }
    else
        SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

    list->label.scale = INTERFACE_SCALE;
    list->label.x = dst.x / INTERFACE_SCALE + 8;
    list->label.y = dst.y / INTERFACE_SCALE + 25;
    list->label.text = g_defaultPresets[list->preset].name;
    label_update(renderer, (Component*)&list->label);
    return true;
}