#include <Maps.h>
#include <States.h>
#include <CMath.h>

#include <maps/HideAndSeek2.h>
#include <maps/RavineMist.h>
#include <maps/DotDotDot.h>
#include <maps/YouCantRun.h>
#include <maps/LimpCity.h>
#include <maps/NotPerfect.h>
#include <maps/KindAndFair.h>
#include <maps/Act9.h>
#include <maps/NastyParadise.h>
#include <maps/PricelessFreedom.h>
#include <maps/VolcanoValley.h>
#include <maps/Hill.h>
#include <maps/MajinForest.h>
#include <maps/TortureCave.h>
#include <maps/DarkTower.h>
#include <maps/HauntingDream.h>
#include <maps/FartZone.h>

Map g_mapList[19] =
{
	{ "Hide and Seek 2",	{ hs2_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 22 }, //0
	{ "Ravine Mist",		{ rmz_init, rmz_tick, rmz_tcpmsg, rmz_left, map_uninit }, 1, 27 }, //1
	{ "...",				{ dot_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 25 }, //2
	{ "Desert Town",		{ map_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 25 }, //3
	{ "You Can't Run",		{ ycr_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 27 }, //4
	{ "Limp City",			{ lc_init,	map_tick, lc_tcpmsg,  map_left, map_uninit }, 1, 23 }, //5
	{ "Not Perfect",		{ np_init,  map_tick, map_tcpmsg, map_left, map_uninit }, 1, 59 }, //6
	{ "Kind and Fair",		{ kaf_init, map_tick, kaf_tcpmsg, map_left, map_uninit }, 1, 31 }, //7
	{ "Act 9",				{ act9_init,map_tick, map_tcpmsg, map_left, map_uninit }, 1, 38 }, //8
	{ "Nasty Paradise",		{ nap_init, map_tick, nap_tcpmsg, map_left, map_uninit }, 1, 26 }, //9
	{ "Priceless Freedom",	{ pf_init,	map_tick, pf_tcpmsg,  map_left, map_uninit }, 1, 38 }, //10
	{ "Volcano Valley",		{ vv_init,	map_tick, vv_tcpmsg,  map_left, map_uninit }, 1, 27 }, //11
	{ "Hill",				{ hill_init,map_tick, map_tcpmsg, map_left, map_uninit }, 1, 26 }, //12
	{ "Majin Forest",		{ maj_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 20 }, //13
	{ "Hide and Seek",		{ map_init, map_tick, map_tcpmsg, map_left, map_uninit }, 1, 21 }, //14
	{ "Torture Cave",		{ tc_init,  map_tick, map_tcpmsg, map_left, map_uninit }, 1, 27 }, //15
	{ "Dark Tower",			{ dt_init,  map_tick, dt_tcpmsg,  map_left, map_uninit }, 1, 31 }, //16
	{ "Haunting Dream",		{ hd_init,  map_tick, hd_tcpmsg,  map_left, map_uninit }, 1, 31 }, //17
	{ "Fart Zone",			{ ft_init,	map_tick, ft_tcpmsg,  map_left, map_uninit }, 1, 15 }, //18
};

bool map_init(Server* server)
{
	RAssert(map_time(server, 3 * TICKSPERSEC, 20));
	RAssert(map_ring(server, 5));
	 
	return true;
}

bool map_uninit(Server* server)
{
	return true;
}

bool map_tick(Server* server)
{
	if (server->game.time >= TICKSPERSEC)
	{
		if (server->game.time_sec == TICKSPERSEC)
			game_bigring(server, BS_DEACTIVATED);

		if (server->game.time_sec == TICKSPERSEC - 10)
			game_bigring(server, BS_ACTIVATED);
	}

	return true;
}

bool map_tcpmsg(PeerData* v, Packet* packet)
{
	return true;
}
bool map_left(PeerData* v)
{
	return true;
}

bool map_time(Server* server, double seconds, float mul)
{
	server->game.time_sec = (uint16_t)(seconds + (((server_ingame(server) - 1) * mul)));
	return true;
}

bool map_ring(Server* server, int ringcoff)
{
	server->game.ring_coff = ringcoff;
	if (server_ingame(server) > 3)
		server->game.ring_coff--;
	
	if (server->game.ring_coff < 1)
		server->game.ring_coff = 1;
	return true;
}