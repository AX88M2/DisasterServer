#include <ui/Components.h>

bool button_update(SDL_Renderer* renderer, struct _Component* component)
{
    Button* button = (Button*)component;
	SDL_FRect src = { button->x, button->y, button->w, button->h };
	SDL_FRect dst = { button->d_x * INTERFACE_SCALE, button->d_y * INTERFACE_SCALE, button->d_w * INTERFACE_SCALE, button->d_h * INTERFACE_SCALE };

    float mouse_x, mouse_y;
    float scale_x, scale_y;
    Uint32 flags = SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_GetRenderScale(renderer, &scale_x, &scale_y);

    mouse_x /= scale_x;
    mouse_y /= scale_y;

    if(mouse_x >= dst.x && mouse_y >= dst.y && mouse_x < dst.x + dst.w && mouse_y < dst.y + dst.h)
    {
        src.x += src.w;

        if(flags & SDL_BUTTON_MASK(1))
        {
            if(!button->clicked && button->cb)
            {
                button->clicked = true;

                if (!button->cb(component))
                    return false;
            }
            dst.y += 2;
        }
        else
            button->clicked = false;
    }
    else
        button->clicked = false;

	SDL_RenderTexture(renderer, g_textureSheet, &src, &dst);
    return true;
}
