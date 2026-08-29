#include <ui/Resources.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Resources
#include <ui/Spritesheet.h>

SDL_Texture* g_textureSheet = NULL;
SDL_Texture* texture_load(SDL_Renderer* renderer, const void* data, unsigned int length)
{
	SDL_RWops* rw = SDL_RWFromConstMem(data, (int)length);
	SDL_Surface* img = IMG_Load_RW(rw, 0);
	if (!img)
		return NULL;

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, img);
	SDL_FreeSurface(img);

	return texture;
}

bool resources_load(SDL_Renderer* renderer)
{
	if (!(g_textureSheet = texture_load(renderer, Spritesheet_png, Spritesheet_png_len))) 
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL ERROR", SDL_GetError(), NULL);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to load " "g_textureSheet" "!\nPress OK to fallback to console mode.", NULL); 
		return 0;
	}
	return true;
}
