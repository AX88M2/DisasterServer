#include "Config.h"
#include <string.h>
#include <ui/Main.h>
#include <ui/Components.h>
#include <ui/Presets.h>
#include <io/Threads.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Lib.h>
#include <Maps.h>
#include <Server.h>
#include <States.h>
#include <stdbool.h>
#include <stdio.h>

int 	lobby = 0;
enum
{
	UST_MAIN,
	UST_OPTIONS,
	UST_PLAYERS,
	UST_INFO
}		state;
char	text_buffer[40][256];
char 	info_text[1024];
int		text_index = 0;
int		text_length = 0;
int 	g_mouseWheel = 0;
int 	g_pmouseWheel = 0;

void init_info();
bool main_menu(Component* component);
bool options_menu(Component* component);
bool players_menu(Component* component);
bool info_menu(Component* component);

bool back_to_lobby(Component* component);
bool practice_mode(Component* component);
bool force_exe_win(Component* component);
bool force_surv_win(Component* component);

bool map_list_changed(Component* component);
bool map_list_reset(Component* component);

Component* main_components[] =
{
	(Component*)&(ImageCreate(0, 0, 480, 270, 0, 272, 480, 270)),
	(Component*)&(ImageCreate(16, 48, 276, 176, 128, 0, 272, 176)),
	(Component*)&(ImageCreate(199, 10, 64, 16, 1829, 0, 64, 16)),

	(Component*)&(ButtonCreate(59, 249, 32, 8, main_menu, 480, 400, 32, 8)),
	(Component*)&(ButtonCreate(150, 249, 54, 8, options_menu, 480, 408, 54, 8)),
	(Component*)&(ButtonCreate(268, 249, 53, 8, players_menu, 480, 416, 53, 8)),
	(Component*)&(ButtonCreate(386, 249, 29, 8, info_menu, 480, 424, 29, 8)),

	(Component*)&(ButtonCreate(320, 106, 128, 20, back_to_lobby, 400, 0, 128, 20)),
	(Component*)&(ButtonCreate(320, 136, 128, 20, practice_mode, 400 + 256, 0, 128, 20)),
	(Component*)&(ButtonCreate(320, 166, 128, 20, force_exe_win, 400 + 512, 0, 128, 20)),
	(Component*)&(ButtonCreate(320, 196, 128, 20, force_surv_win, 400 + 512 + 256, 0, 128, 20)),
};

Component* options_components[] =
{
	(Component*)&(MapListCreate(0, 38, 288, 195, map_list_changed)),
	(Component*)&(ImageCreate(0, 0, 480, 270, 2256, 0, 480, 270)),
	(Component*)&(ImageCreate(184, 10, 103, 16, 1925, 0, 103, 16)),

	(Component*)&(ButtonCreate(59, 249, 32, 8, main_menu, 480, 400, 32, 8)),
	(Component*)&(ButtonCreate(150, 249, 54, 8, options_menu, 480, 408, 54, 8)),
	(Component*)&(ButtonCreate(268, 249, 53, 8, players_menu, 480, 416, 53, 8)),
	(Component*)&(ButtonCreate(386, 249, 29, 8, info_menu, 480, 424, 29, 8)),

	(Component*)&(MapListPresetCreate(320, 48, 128, 40)),
	(Component*)&(ButtonCreate(320, 96, 128, 20, map_list_reset, 3540, 0, 128, 20)),
	(Component*)&(PingLimitCreate(320, 125, 128, 40)),
	(Component*)&(TButtonCreate(320, 172, 128, 20, (ButtonCallback)config_save, &g_config.anticheat, true, 3120, 0, 128, 20)),
	(Component*)&(TButtonCreate(320, 200, 128, 20, (ButtonCallback)config_save, &g_config.pride, false, 2736, 0, 128, 20)),
};

Component* players_components[] =
{
	(Component*)&(ImageCreate(0, 0, 480, 270, 0, 272, 480, 270)),
	(Component*)&(ImageCreate(179, 10, 112, 16, 2032, 0, 112, 16)),

	(Component*)&(PlayerListCreate(16, 48)),
	(Component*)&(PlayerListConfigCreate(168, 48, playerlist_op_update)),
	(Component*)&(PlayerListConfigCreate(320, 48, playerlist_bans_update)),

	(Component*)&(ButtonCreate(59, 249, 32, 8, main_menu, 480, 400, 32, 8)),
	(Component*)&(ButtonCreate(150, 249, 54, 8, options_menu, 480, 408, 54, 8)),
	(Component*)&(ButtonCreate(268, 249, 53, 8, players_menu, 480, 416, 53, 8)),
	(Component*)&(ButtonCreate(386, 249, 29, 8, info_menu, 480, 424, 29, 8)),
};

Component* info_components[] =
{
	(Component*)&(ImageCreate(0, 0, 480, 270, 0, 272, 480, 270)),
	(Component*)&(ImageCreate(208, 10, 55, 16, 2172, 0, 55, 16)),
	(Component*)&(LabelCreate(4, 38 + 4, info_text, 2)),

	(Component*)&(ButtonCreate(59, 249, 32, 8, main_menu, 480, 400, 32, 8)),
	(Component*)&(ButtonCreate(150, 249, 54, 8, options_menu, 480, 408, 54, 8)),
	(Component*)&(ButtonCreate(268, 249, 53, 8, players_menu, 480, 416, 53, 8)),
	(Component*)&(ButtonCreate(386, 249, 29, 8, info_menu, 480, 424, 29, 8)),
};

void log_msg(const char* type, const char* message)
{
	char msg[512];

	if (strcmp(type, DEBUG_TYPE) == 0)
		snprintf(msg, 512, "|%s~", message);
	else if (strcmp(type, WARN_TYPE) == 0)
		snprintf(msg, 512, "`%s~", message);
	else if (strcmp(type, ERROR_TYPE) == 0)
		snprintf(msg, 512, "\\%s~", message);
	else
		snprintf(msg, 512, "~%s", message);

	if (text_length > 39)
	{
		for (int i = 1; i < 40; i++)
			strncpy(text_buffer[i - 1], text_buffer[i], 255);
	}

	strncpy(text_buffer[text_index], msg, 255);

	if (text_index < 39)
		text_index++;

	if (text_length < 40)
		text_length++;
}

int server_loop(void)
{
	if (!disaster_init())
		return 1;

	return disaster_run();
}

int console_loop(void)
{
	SDL_Quit();

#ifdef WIN32
	AllocConsole();
	(void)freopen("CONOUT$", "w", stdout);
#endif

	return server_loop();
}

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "--nogui") == 0)
				return console_loop();
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		char msg[1024];
		snprintf(msg, 1024, "%s\n\nPress OK to fallback to console mode.", SDL_GetError());

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", msg, NULL);
		return console_loop();
	}

	SDL_Window* window;
	SDL_Renderer* renderer;
	if (!(window = SDL_CreateWindow("DisasterServer v" STRINGIFY(BUILD_VERSION) " (Modified)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 480 * INTERFACE_SCALE, 270 * INTERFACE_SCALE, SDL_WINDOW_SHOWN)))
	{
		char msg[1024];
		snprintf(msg, 1024, "%s\n\nPress OK to fallback to console mode.", SDL_GetError());

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", msg, NULL);
		return console_loop();
	}

	if (!(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)))
	{
		char msg[1024];
		snprintf(msg, 1024, "%s\n\nPress OK to fallback to console mode.", SDL_GetError());

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", msg, NULL);
		return console_loop();
	}

	if (!resources_load(renderer))
		return console_loop();

	init_info();
	log_hook(log_msg);
	SDL_RenderSetLogicalSize(renderer, 480 * INTERFACE_SCALE, 270 * INTERFACE_SCALE);
	SDL_RenderSetVSync(renderer, true);

	Label label;
	Thread thr;
	ThreadSpawn(thr, server_loop, NULL);

	bool running = true;
	float bg = 0;

	while (running)
	{
		SDL_Event ev;
		while (SDL_PollEvent(&ev))
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				running = false;
				break;

			case SDL_MOUSEWHEEL:
				g_mouseWheel = ev.wheel.y;
				break;
			}
		}
		
		SDL_RenderClear(renderer);
		{
			SDL_Rect src = (SDL_Rect){ 480 + ((int)bg / 3 % 28) * 96, 446, 96, 96 };
			for(int i = 0; i < 6 * INTERFACE_SCALE; i++)
			{
				for(int j = 0; j < 3 * INTERFACE_SCALE; j++)
				{
					SDL_Rect dst = (SDL_Rect){ -((int)bg / 2 % 96) + i * 96, j * 96, 96, 96 };
					SDL_RenderCopy(renderer, g_textureSheet, &src, &dst);
				}
			}

			switch (state)
			{
			case UST_MAIN:
			{
				for (int i = 0; i < sizeof(main_components) / sizeof(Component*); i++)
					main_components[i]->update(renderer, main_components[i]);

				for (int i = 0; i < text_length; i++)
				{
					label = LabelCreate(23 * 2, 55 * 2 + i * 8, text_buffer[i], 1);
					label.update(renderer, (Component*)&label);
				}

				break;
			}

			case UST_OPTIONS:
			{
				for (int i = 0; i < sizeof(options_components) / sizeof(Component*); i++)
					options_components[i]->update(renderer, options_components[i]);

				break;
			}

			case UST_PLAYERS:
			{
				for (int i = 0; i < sizeof(players_components) / sizeof(Component*); i++)
					players_components[i]->update(renderer, players_components[i]);

				break;
			}

			case UST_INFO:
			{
				for (int i = 0; i < sizeof(info_components) / sizeof(Component*); i++)
					info_components[i]->update(renderer, info_components[i]);

				break;
			}
			}

			g_mouseWheel = 0;
			bg += 0.64;
		}
		SDL_RenderPresent(renderer);
		SDL_Delay(32);
	}

	return 0;
}

void init_info()
{
	char compiler[128];

	#ifdef __clang__
        snprintf(compiler, 128, "clang %d.%d.%d\n", __clang_major__, __clang_minor__, __clang_patchlevel__);
    #elif defined(__GNUC__)
        snprintf(compiler, 128, "gcc %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    #elif defined(_MSC_VER)
        snprintf(compiler, 128, "msvc %d", _MSC_VER);
    #else
        snprintf(compiler, 128, "unknown");
    #endif

	SDL_version sdl_ver;
	SDL_GetVersion(&sdl_ver);
	const SDL_version* img_ver = IMG_Linked_Version();

	snprintf(info_text, 1024, INFO_TEXT, compiler, sdl_ver.major, sdl_ver.minor, sdl_ver.patch, img_ver->major, img_ver->minor, img_ver->patch);
}

bool main_menu(Component* component)
{
	state = UST_MAIN;
	return true;
}

bool options_menu(Component* component)
{
	state = UST_OPTIONS;
	return true;
}

bool players_menu(Component* component)
{
	state = UST_PLAYERS;
	return true;
}

bool info_menu(Component* component)
{
	state = UST_INFO;
	return true;
}

bool back_to_lobby(Component* component)
{
	Server* server = disaster_get(lobby);

	if(!server)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Server is not running (Check logs in Info tab)!", NULL);
		return false;
	}

	MutexLock(server->state_lock);
	{
		if(server->state != ST_LOBBY && !lobby_init(server))
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to return to lobby: lobby_init() returned FALSE", NULL);
	}
	MutexUnlock(server->state_lock);
	return true;
}

bool practice_mode(Component* component)
{
	Server* server = disaster_get(lobby);

	if(!server)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Server is not running (Check logs in Info tab)!", NULL);
		return false;
	}

	MutexLock(server->state_lock);
	{
		if(server->state == ST_LOBBY && server_ingame(server) > 1)
		{
			if(!charselect_init(20, server))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to set practice mode: charselect_init() returned FALSE", NULL);
			}
		}
	}
	MutexUnlock(server->state_lock);

	return true;
}

bool force_exe_win(Component* component)
{
	Server* server = disaster_get(lobby);

	if(!server)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Server is not running (Check logs in Info tab)!", NULL);
		return false;
	}

	MutexLock(server->state_lock);
	{
		if(server->state == ST_GAME)
		{
			if(!game_end(server, ED_EXEWIN, false))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to set exe win: game_end() returned FALSE", NULL);
			}
		}
	}
	MutexUnlock(server->state_lock);

	return true;
}

bool force_surv_win(Component* component)
{
	Server* server = disaster_get(lobby);

	if(!server)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Server is not running (Check logs in Info tab)!", NULL);
		return false;
	}

	MutexLock(server->state_lock);
	{
		if(server->state == ST_GAME)
		{
			if(!game_end(server, ED_SURVWIN, false))
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to set surv win: game_end() returned FALSE", NULL);
			}
		}
	}
	MutexUnlock(server->state_lock);

	return true;
}

bool map_list_changed(Component* component)
{
	MapListPreset* preset_list = (MapListPreset*)options_components[7];
	preset_list->preset = PRESET_CUSTOM;

	for (int i = 0; i < PRESET_COUNT; i++)
	{
		if (memcmp(g_config.map_list, g_defaultPresets[i].values, sizeof(g_config.map_list)) == 0)
		{
			preset_list->preset = i;
			break;
		}
	}

	return true;
}

bool map_list_reset(Component* component)
{
	MapListPreset* preset_list = (MapListPreset*)options_components[7];
	preset_list->preset = PRESET_CUSTOM;

	MutexLock(g_config.map_list_lock);
	memset(g_config.map_list, 0, sizeof(g_config.map_list));
	MutexUnlock(g_config.map_list_lock);
	return true;
}

void ui_update_playerlist(Server* server)
{
	if(!server) // just in case
		return;

	if(server->id != 0) // we only update 0 server
		return;

	PlayerList* list = (PlayerList*)players_components[2];
	for(size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* player = (PeerData*)server->peers.ptr[i];
		if(!player)
		{
			memset(&list->peers[i], 0, sizeof(list->peers[i]));
			continue;
		}
		
		memcpy(&list->peers[i], server->peers.ptr[i], sizeof(list->peers[i]));
	}
}

bool ui_update_delete(Component* component)
{
	DeleteButton* delete = (DeleteButton*)component;
	if (cJSON_HasObjectItem(delete->root, delete->key))
	{
		cJSON_DeleteItemFromObject(delete->root, delete->key);

		if (!collection_save(delete->collection, delete->root))
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error (Report to dev)", "Failed to save collection!", NULL);
	}
	return false;
}

bool ui_button_op(struct _Component* component)
{
	PlayerButton* button = (PlayerButton*)component;
	Server* server = disaster_get(lobby);

	bool res = true;
	MutexLock(server->state_lock);
	{
		for(size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* peer = (PeerData*)server->peers.ptr[i];
			if(!peer)
				continue;

			if(peer->id != button->peer.id)
				continue;
			
			if(strcmp(peer->nickname.value, button->peer.nickname.value) != 0)
				continue;

			if(strcmp(peer->udid.value, button->peer.udid.value) != 0)
				continue;

			if (peer->op)
				break;
			
			peer->op = true;

			server_send_msg(server, peer->peer, CLRCODE_GRN "you're an operator now");
			res = op_add(peer->nickname.value, peer->ip.value);
		}
	}
	MutexUnlock(server->state_lock);
	
	RAssert(res);
	return false;
}

bool ui_button_kick(struct _Component* component)
{
	PlayerButton* button = (PlayerButton*)component;
	Server* server = disaster_get(lobby);

	bool res = true;
	MutexLock(server->state_lock);
	{
		for(size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* peer = (PeerData*)server->peers.ptr[i];
			if(!peer)
				continue;

			if(peer->id != button->peer.id)
				continue;
			
			if(strcmp(peer->nickname.value, button->peer.nickname.value) != 0)
				continue;

			if(strcmp(peer->udid.value, button->peer.udid.value) != 0)
				continue;

			server_disconnect(server, peer->peer, DR_KICKEDBYHOST, NULL);
			res = timeout_set(peer->nickname.value, peer->udid.value, peer->ip.value, time(NULL) + 5);
		}
	}
	MutexUnlock(server->state_lock);

	RAssert(res);
	return false;
}

bool ui_button_ban(struct _Component* component)
{
	PlayerButton* button = (PlayerButton*)component;
	Server* server = disaster_get(lobby);

	bool res = true;
	MutexLock(server->state_lock);
	{
		for(size_t i = 0; i < server->peers.capacity; i++)
		{
			PeerData* peer = (PeerData*)server->peers.ptr[i];
			if(!peer)
				continue;

			if(peer->id != button->peer.id)
				continue;
			
			if(strcmp(peer->nickname.value, button->peer.nickname.value) != 0)
				continue;

			if(strcmp(peer->udid.value, button->peer.udid.value) != 0)
				continue;

			server_disconnect(server, peer->peer, DR_BANNEDBYHOST, NULL);
			res = ban_add(peer->nickname.value, peer->udid.value, peer->ip.value);
		}
	}
	MutexUnlock(server->state_lock);

	RAssert(res);
	return false;
}