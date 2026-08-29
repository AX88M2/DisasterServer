#include <ui/Components.h>

bool image_update(SDL_Renderer* renderer, struct _Component* component)
{
	Image* image = (Image*)component;

	SDL_Rect src = { image->x, image->y, image->w, image->h };
	SDL_Rect dst = { image->d_x * INTERFACE_SCALE, image->d_y * INTERFACE_SCALE, image->d_w * INTERFACE_SCALE, image->d_h * INTERFACE_SCALE };
	SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);

	return true;
}
