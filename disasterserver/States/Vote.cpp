#include "Vote.hpp"

#include <algorithm>

#include "Client.hpp"
#include "Server.hpp"

using namespace DisasterServer;

Vote::Vote(Server *server) : server(server) {
}

Vote::~Vote() = default;

bool Vote::init(VoteType type, clientId id) {
    ongoing = false;
    votes.clear();
    auto &players = server->getClients();
    this->type = type;

    for (auto &c: server->getClients()) {
        c->setCanVote(c->getId() != id);
    }
    this->votedTotal = std::ranges::count_if(players, [](const auto& peer) { return peer->isCanVote(); });

    if (votedTotal <= 1) {
        return false;
    }

    this->ongoing = true;
    this->countdown = 20 * TICKSPERSEC;
    return true;
}

VoteState Vote::add(Client & voter) {
    if (this->votes.contains(voter.getId())) {
        return VoteState::ALREADY_VOTED;
    }

    votes.insert(voter.getId());

    if (votes.size() >= votedTotal) {
        return VoteState::FULL;
    }

    return VoteState::SUCCESS;
}

bool Vote::tick() {
    if (!ongoing) {
        return false;
    }

    countdown -= server->getDelta();

    if (countdown <= 0) {
        return false;
    }

    return true;
}

bool Vote::check() {
    return votes.size() * 4 > votedTotal;
}