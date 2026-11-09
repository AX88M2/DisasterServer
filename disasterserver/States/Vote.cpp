#include "Vote.hpp"

#include <algorithm>

#include "Client.hpp"
#include "Server.hpp"

using namespace DisasterServer;

Vote::Vote(Server &server) : server(server) {
}

Vote::~Vote() = default;

bool Vote::init(const VoteType type_, const clientId id) {
    this->ongoing = false;
    this->type = type_;

    votes.clear();

    auto &clients = server.getClients();

    for (const auto &client: server.getClients()) {
        client->setCanVote(client->getId() != id);
    }

    this->votedTotal = std::ranges::count_if(clients, [](const auto& client) { return client->isCanVote(); });

    if (votedTotal <= 1) {
        return false;
    }

    this->ongoing = true;
    this->countdown.start(20);
    return true;
}

VoteState Vote::add(const Client & voter) {
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

    const auto result = countdown.tick(server.getDelta());

    if (result == TickResult::Finished) {
        return false;
    }

    return true;
}

bool Vote::check() {
    return votes.size() * 2 > (votedTotal / 2.0);
}