#include "StateManager.hpp"
#include "Server.hpp"

using namespace DisasterServer;

StateManager::StateManager(Server *server) : server(server), lobby(server, this) {
}

StateManager::~StateManager() = default;

bool StateManager::state_joined(Client &peer) {
    Packet packet(PacketType::SERVER_LOBBY_EXE_CHANCE);
    packet.write<uint8_t>(peer.getExeChance());
    packet.send(peer, true);

    Packet playerJoined(PacketType::SERVER_PLAYER_JOINED);
    playerJoined.write<uint16_t>(peer.getId());
    playerJoined.writeString(peer.getNickname());
    playerJoined.write<uint8_t>(peer.getLobbyIcon());
    playerJoined.write<uint8_t>(peer.getPet());
    this->server->broadcast_ex(playerJoined, true, peer.getId());

    switch (state) {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE: {
            return lobby.joined(peer);
        }
        case States::GAME: {
            return true;
        }
        case States::RESULTS: {
            break;
        }
        default: break;
    }

    return true;
}

void StateManager::state_tick() {
    switch (state)
    {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE:
            lobby.tick();
            break;

        case States::GAME:
            //game_state_tick(server);
            break;

        case States::RESULTS:
            //results_state_tick(server);
            break;
    }
}

bool StateManager::state_handle(Client &peer, Packet &packet) {

    switch (state) {
        case States::LOBBY:
        case States::CHARSELECT:
        case States::MAPVOTE: {
            lobby.handle(peer, packet);
            break;
        }
        case States::GAME: {
            break;
        }
        case States::RESULTS: {
            break;
        }
        default: break;
    }

    return true;
}

void StateManager::setState(States state) {
    this->state = state;
}

void StateManager::state_left(Client &peer) {

}
