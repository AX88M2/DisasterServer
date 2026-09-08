#include <Player.h>
#include <Server.h>
#include <Zone.h>

bool player_add_error(Server* server, PeerData* v, uint16_t by)
{
	v->plr.errors += by;

	Debug("%d error is now %d", v->id, v->plr.errors);
	if (v->plr.errors >= 30000)
	{
		server_disconnect(server, v->peer, DR_OTHER, "Fun fact: dial-up internet connection was relevant 20 years ago!");
		return false;
	}

	return true;
}

void player_check_zone(Server* server, PeerData* v)
{
	if (!g_config.anticheat)
		return;

	if (!server->game.started)
		return;

	if (v->plr.pos.x == 0 && v->plr.pos.y == 0)
		return;

	const Zone* zones = g_mapZone[server->game.map];
	const size_t zonesSize = g_mapZoneSize[server->game.map];
	for (size_t i = 0; i < zonesSize; i++)
	{
		const Zone zone = zones[i];
		if (v->plr.pos.x >= zone.x && v->plr.pos.y >= zone.y && v->plr.pos.x <= zone.x + zone.w && v->plr.pos.y <= zone.y + zone.h)
		{
			Debug("%d is inside invalid area %d", v->id, i);
			if (!player_add_error(server, v, server->game.map == 6 ? 30 : 1000))
				return;
		}
	}
}
