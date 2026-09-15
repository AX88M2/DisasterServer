#include "ResultsState.hpp"

#include "LobbyState.hpp"

using namespace DisasterServer;

ResultsState::ResultsState(Server &server, StateController &stateController) : State(server, stateController) {
}

void ResultsState::init() {
    Debug("Attepting to enter DisasterServer::ResultsState...");


    Info("Server is now in {}Results", CLRCODE_PUR);
}

bool ResultsState::playerJoined(Client &client) {
    return State::playerJoined(client);
}

bool ResultsState::playerLeaved(Client &client) {
    return State::playerLeaved(client);
}

void ResultsState::tick() {
    State::tick();
}

bool ResultsState::handle(Client &client, Packet &packet) {
    return State::handle(client, packet);
}
