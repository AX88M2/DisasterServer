#ifndef RMZSLIME_H
#define RMZSLIME_H
#include "../States.h"

bool slug_init(Server* server, Entity* entity);
bool slug_tick(Server* server, Entity* entity);
bool slug_uninit(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	float sX, sY;
	enum
	{
		SLUG_NONERIGHT,
		SLUG_NONELEFT,
		SLUG_RINGRIGHT,
		SLUG_RINGLEFT,
		SLUG_REDRINGRIGHT,
		SLUG_REDRINGLEFT
	} state;

	enum
	{
		SLUG_NORING,
		SLUG_RING,
		SLUG_REDRING
	} ring;
} Slug;
#define MakeSlug(x, y) ((Slug) { MakeEntity("slug", x, y) slug_init, slug_tick, slug_uninit, 0.f, 0.f, 0, 0 })

bool slugspawn_tick(Server* server, Entity* entity);

typedef struct
{
	ENTITY_BODY

	double	 offset;
	double	 timer;
	uint16_t slug;
} SlugSpawner;
#define MakeSlugSpawn(x, y) ((SlugSpawner) { MakeEntity("slugspawn", x, y) NULL, slugspawn_tick, NULL, (rand() % 10) * TICKSPERSEC, 0, 0 })

#endif