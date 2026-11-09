#pragma once

#include "Client.hpp"
#include "Util/Countdown.hpp"

namespace DisasterServer
{
    class Server;

    enum class VoteType
    {
        KICK,
        PRACTICE
    };

    enum class VoteState
    {
        SUCCESS = 1,
        NOT_ONGOING = 2,
        NOT_ALLOWED = 3,
        ALREADY_VOTED = 0,
        FULL = -1,
    };

    class Vote {
        Server &server;

        VoteType type = VoteType::KICK;
        bool ongoing = false;

        std::unordered_set<clientId> votes;
        size_t votedTotal = 0;
        Countdown countdown;

    public:
        Vote(Server &server);
        ~Vote();

        bool init(VoteType type_, clientId id);
        VoteState add(const Client &voter);
        bool tick();
        bool check();

        void setOnGoing(bool flag) { ongoing = flag; }
        bool isOnGoing() const { return ongoing; }
        VoteType getCurrentVoteType() const { return type; }
        size_t getVoteTotal() const { return votedTotal; }
        size_t getVoteCount() const { return votes.size(); }
    };
}