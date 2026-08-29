#include <ui/Components.h>
#include <UTF8.h>

bool label_update(SDL_Renderer* renderer, struct _Component* component)
{
	Label* label = (Label*)component;
	int x = label->x;
	int y = label->y;
	SDL_Color clr = COLOR_WHITE;

	for (int i = 0; i < utf8_strlen(label->text); i++)
	{
		utf8_char c = utf8_tolower(utf8_get(label->text, i));
		
		int ind;
		switch (c)
		{
		case '\n':
			x = label->x;
			y += 8;
			continue;

		case ' ':
			x += 5;
			continue;

		case '	':
			x += 10;
			continue;

		case '-':
		case '_':
			ind = 26;
			break;

		case ',':
			ind = 27;
			break;

		case '1':
			ind = 28;
			break;

		case '2':
			ind = 29;
			break;

		case '3':
			ind = 30;
			break;

		case '4':
			ind = 31;
			break;

		case '5':
			ind = 32;
			break;

		case '6':
			ind = 33;
			break;

		case '7':
			ind = 34;
			break;

		case '8':
			ind = 35;
			break;

		case '9':
			ind = 36;
			break;

		case '0':
			ind = 37;
			break;

		case '!':
		case '.':
			ind = 38;
			break;

		case '\'':
			ind = 39;
			break;

		case ':':
			ind = 40;
			break;

		case '(':
		case '[':
		case '<':
			ind = 41;
			break;

		case ')':
		case ']':
		case '>':
			ind = 42;
			break;

		case '%':
			ind = 43;
			break;

		case '+':
			ind = 77;
			break;

		case '\\':
			clr = COLOR_RED;
			continue;

		case '@':
			clr = COLOR_GRN;
			continue;

		case '&':
			clr = COLOR_PUR;
			continue;

		case '/':
			clr = COLOR_BLU;
			continue;

		case '|':
			clr = COLOR_GRA;
			continue;

		case '`':
			clr = COLOR_YLW;
			continue;

		case '~':
			clr = COLOR_WHITE;
			continue;

		case 0x2116:
			clr = COLOR_ORG;
			continue;

		default:
			if(c >= 'a' && c <= 'z')
				ind = c - 97;
			else if(c >= 0x0430 && c <= 0x044F)
				ind = c - 0x0430 + 45;
			else
				ind = 44;
			break;
		}

		SDL_SetTextureColorMod(g_textureSheet, clr.r, clr.g, clr.b);
		SDL_Rect src = (SDL_Rect){ 480 + ind * 8, 432, 8, 6 };
		SDL_Rect dst = (SDL_Rect){ x * label->scale, y * label->scale, 8 * label->scale, 6 * label->scale };
		SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);
		x += 6;

		switch (c)
		{
		case 'w':
		case 'm':
		case 'x':
		case 'n':
		case 0x043C:
		case 0x0434:
		case 0x0438:
		case 0x0439:
		case 0x044E:
		case 0x044C:
		case 0x043B:
		case 0x0448:
		case 0x0449:
		case 0x0446:
		case 0x0436:
			x += 2;
			break;
		}
	}

	SDL_SetTextureColorMod(g_textureSheet, 255, 255, 255);
	return true;
}
