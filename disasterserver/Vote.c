#include <Vote.h>
#include <States.h>

bool vote_init(Server* server, Vote* vote, VoteType type, uint16_t id)
{
	vote->votecnt = 0;
	vote->votetotal = 0;
	vote->type = type;

	for (size_t i = 0; i < server->peers.capacity; i++)
	{
		PeerData* peer = (PeerData*)server->peers.ptr[i];
		if (!peer)
			continue;
		
		peer->can_vote = (peer->id != id);
		
		if(peer->can_vote)
			vote->votetotal++;
	}

	if (vote->votetotal <= 1)
		return false;

	vote->ongoing = true;
	vote->countdown = 20 * TICKSPERSEC;
	memset(vote->votes, 0, 6 * sizeof(uint8_t));
	return true;
}

VoteState vote_add(Vote* vote, uint16_t id)
{
	for (int i = 0; i < vote->votecnt; i++)
	{
		if (vote->votes[i] == id)
			return VOTE_ALREADYVOTED;
	}

	vote->votes[vote->votecnt++] = id;
	if (vote->votecnt >= vote->votetotal)
		return VOTE_FULL;

	return VOTE_SUCCESS;
}

bool vote_tick(Server* server, Vote* vote)
{
	vote->countdown -= server->delta;
	if (vote->countdown <= 0)
		return false;

	return true;
}

bool vote_check(Vote* vote)
{
	return vote->votecnt > (vote->votetotal / 2.0);
}
