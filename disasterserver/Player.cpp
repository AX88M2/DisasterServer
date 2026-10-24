#include "Player.hpp"

#include "Application.hpp"
#include "Client.hpp"
#include "Core/Log.hpp"

using namespace DisasterServer;

Player::Player() = default;

Player::~Player() = default;

bool Player::addError(Client &client, uint16_t by) {
    errors += by;

    Debug("{} error is now {}", client.getId(), errors);

    if (errors >= 30000) {
        client.disconnect(DisconnectReason::OTHER, "Fun fact: dial-up internet connection was relevant 20 years ago!");
        return false;
    }

    return true;
}