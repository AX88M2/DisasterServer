#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <Maps.h>
#include <ui/Resources.h>

#define COMPONENT_BODY int x, y, w, h; UpdateCallback update
#define COLOR_WHITE (SDL_Color)	{ 255, 255, 255, 255 }
#define COLOR_RED	(SDL_Color) { 194, 0, 55, 255 }
#define COLOR_GRN	(SDL_Color) { 15, 255, 57, 255 }
#define COLOR_PUR	(SDL_Color) { 184, 36, 255, 255 }
#define COLOR_BLU	(SDL_Color) { 93, 103, 255, 255 }
#define COLOR_GRA	(SDL_Color) { 100, 100, 100, 255 }
#define COLOR_YLW	(SDL_Color) { 255, 219, 0, 255 }
#define COLOR_ORG	(SDL_Color) { 234, 96, 20, 255 }
#define INTERFACE_SCALE 2

extern int g_mouseWheel;

struct _Component;
typedef bool (*UpdateCallback)(SDL_Renderer*, struct _Component*);
typedef struct _Component
{
	COMPONENT_BODY;
} Component;

typedef struct
{
	COMPONENT_BODY;
	const char* text;
	int scale;
} Label;
#define LabelCreate(x, y, text, scale) (Label) { x, y, 0, 0, label_update, text, scale }
bool label_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	int d_x, d_y, d_w, d_h;
} Image;
#define ImageCreate(x, y, w, h, s_x, s_y, s_w, s_h) (Image) { s_x, s_y, s_w, s_h, image_update, x, y, w, h }
bool image_update(SDL_Renderer* renderer, struct _Component* component);

typedef bool (*ButtonCallback)(struct _Component* component);
typedef struct
{
	COMPONENT_BODY;
	int d_x, d_y, d_w, d_h;
	ButtonCallback cb;
	bool clicked;
} Button;
#define ButtonCreate(x, y, w, h, cb, s_x, s_y, s_w, s_h) (Button) { s_x, s_y, s_w, s_h, button_update, x, y, w, h, cb, false }
bool button_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	int d_x, d_y, d_w, d_h;
	ButtonCallback cb;
	bool clicked, reverse;
	bool* value;
} ToggleButton;
#define TButtonCreate(x, y, w, h, cb, pointer, reverse, s_x, s_y, s_w, s_h) (ToggleButton) { s_x, s_y, s_w, s_h, tbutton_update, x, y, w, h, cb, false, reverse, pointer }
bool tbutton_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	int d_x, d_y, d_w, d_h;
	ButtonCallback cb;
	bool clicked;
	PeerData peer;
} PlayerButton;

typedef struct
{
	COMPONENT_BODY;
	bool clicked;
	ButtonCallback cb;
	float scroll, target_scroll;
} MapList;
#define MapListCreate(x, y, w, h, cb) (MapList) { x, y, w, h, maplist_update, false, cb, 0, 0 }
bool maplist_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	bool clicked;
	int preset;
	Label label;
} MapListPreset;
#define MapListPresetCreate(x, y, w, h) (MapListPreset) { x, y, w, h, mappreset_update, false, 0 }
bool mappreset_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	bool clicked;
	int preset;
	Label label;
} PingLimit;
#define PingLimitCreate(x, y, w, h) (PingLimit) { x, y, w, h, ping_update, false, 0 }
bool ping_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	bool clicked;
	PeerData peers[7];
} PlayerList;

#define PlayerListCreate(x, y) (PlayerList) { x, y, 144, 176, playerlist_update, false, {0} }
bool playerlist_update(SDL_Renderer* renderer, struct _Component* component);

typedef struct
{
	COMPONENT_BODY;
	int d_x, d_y, d_w, d_h;
	ButtonCallback cb;
	bool clicked;

	const char* collection;
	cJSON* root;
	const char* key;
} DeleteButton;

typedef struct
{
	COMPONENT_BODY;
	bool clicked;
	int page;
} PlayerListConfig;
#define PlayerListConfigCreate(x, y, update) (PlayerListConfig) { x, y, 144, 176, update, false, 0 }
bool playerlist_bans_update(SDL_Renderer* renderer, struct _Component* component);
bool playerlist_op_update(SDL_Renderer* renderer, struct _Component* component);

#endif
