#include <Player.h>
#include <Server.h>

bool player_add_error(Server* server, Player* player, uint16_t by)
{
	player->errors += by;

	Debug("%d error is now %d", player->id, player->errors);
	if (player->errors >= 20000)
	{
		server_disconnect(server, player->id, DR_PACKETSNOTRECV, "Errored too much");
		return false;
	}

	return true;
}