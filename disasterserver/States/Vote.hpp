#ifndef DISASTERSERVER_VOTE_HPP
#define DISASTERSERVER_VOTE_HPP

#include <unordered_set>

#include "Client.hpp"

namespace DisasterServer {
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
        Server *server = nullptr;

        VoteType type = VoteType::KICK;
        bool ongoing = false;

        std::unordered_set<clientId> votes;
        size_t votedTotal = 0;
        double countdown = 0;
    public:
        Vote(Server *server);
        ~Vote();

        bool init(VoteType type, clientId id);
        VoteState add(Client &voter);
        bool tick();
        bool check();

        void setOnGoing(bool flag) { ongoing = flag; }
        bool isOnGoing() const { return ongoing; }
        VoteType getCurrentVoteType() const { return type; }
        size_t getVoteTotal() const { return votedTotal; }
        size_t getVoteCount() const { return votes.size(); }
    };
}

#endif //DISASTERSERVER_VOTE_HPP
