#include <Log.h>
#include <Maps.h>
#include <ui/Resources.h>
#include <ui/Components.h>
#include <CMath.h>

const int low_bound = -(MAP_COUNT/4 * 124 - 124/2 - 194);

bool maplist_update(SDL_Renderer* renderer, struct _Component* component)
{
    MapList* list = (MapList*)component;
	SDL_Rect bounds = { list->x * INTERFACE_SCALE, list->y * INTERFACE_SCALE, list->w * INTERFACE_SCALE, list->h * INTERFACE_SCALE };

    int mouse_x, mouse_y;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_RenderGetScale(renderer, &scale_x, &scale_y);

    mouse_x /= scale_x;
    mouse_y /= scale_y;

    bool mouse_down = false;
    if(flags & SDL_BUTTON(1))
    {
        if(!list->clicked)
        {
            mouse_down = true;
            list->clicked = true;
        }
    }
    else
        list->clicked = false;

    bool in_bounds = mouse_x >= bounds.x && mouse_y >= bounds.y && mouse_x < bounds.x + bounds.w && mouse_y < bounds.y + bounds.h;
    if(in_bounds)
    {
        list->target_scroll += g_mouseWheel * 45;
        if(list->target_scroll >= 0)
            list->target_scroll = 0;

        if(list->target_scroll < low_bound)
            list->target_scroll = low_bound;
    }

    list->scroll = lerp(list->scroll, list->target_scroll, 0.30);

    MutexLock(g_config.map_list_lock);
    for(int i = 0; i < MAP_COUNT; i++)
    {
        SDL_Rect src = { 0, 272, 123, 104 };
        SDL_Rect dst = { 8 + i % 4 * 143, 90 + i / 4 * 124 + list->scroll, 123, 104 };

        src.x = 480 + i * src.w;

        if(in_bounds && mouse_x >= dst.x && mouse_y >= dst.y && mouse_x < dst.x + dst.w && mouse_y < dst.y + dst.h)
        {
            dst.y -= 2;

            if (mouse_down)
            {
                g_config.map_list[i] = !g_config.map_list[i];
                list->cb(component);
            }

            if(flags & SDL_BUTTON(1))
                dst.y += 2;
        }

        const Uint8 color = g_config.map_list[i] ? 255 : 96;
        SDL_SetTextureColorMod(g_textureSheet, color, color, color);
        SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);
    }
    MutexUnlock(g_config.map_list_lock);

    SDL_SetTextureColorMod(g_textureSheet, 255, 255, 255);
    return true;
}
