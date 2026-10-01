#ifndef VOTE_H
#define VOTE_H
#include <Log.h>
#include <stdint.h>

typedef enum
{
	VOTE_KICK,
	VOTE_PRACTICE
} VoteType;

typedef enum
{
	VOTE_SUCCESS = 1,
	VOTE_ALREADYVOTED = 0,
	VOTE_FULL = -1
} VoteState;

typedef struct
{
	VoteType	type;
	bool		ongoing;
	uint16_t	votes[7];
	uint8_t		votecnt;
	uint8_t		votetotal;
	double		countdown;
} Vote;

struct Server;
bool		vote_init(struct Server* server, Vote* vote, VoteType type, uint16_t id);
VoteState	vote_add(Vote* vote, uint16_t id);
bool		vote_tick(struct Server* server, Vote* vote);
bool		vote_check( Vote* vote);

#endif