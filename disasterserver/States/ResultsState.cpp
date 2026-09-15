#include "ResultsState.hpp"

#include "LobbyState.hpp"

using namespace DisasterServer;

ResultsState::ResultsState(Server &server, StateController &stateController) : State(server, stateController) {
}

void ResultsState::init() {
    Debug("Attepting to enter DisasterServer::ResultsState...");


    Info("Server is now in {}Results", CLRCODE_PUR);
}

bool ResultsState::joined(Client &client) {
    return State::joined(client);
}

bool ResultsState::leaved(Client &client) {
    return State::leaved(client);
}

void ResultsState::tick() {
    State::tick();
}

bool ResultsState::handle(Client &client, Packet &packet) {
    return State::handle(client, packet);
}
