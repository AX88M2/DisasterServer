#include "Player.hpp"

using namespace DisasterServer;

Player::Player() = default;

Player::~Player() = default;

void Player::reset() {
    this->ready = 0;
    this->seq = 0;
    this->errors = 0;
    this->exTeleport = 0;
    this->timeout = 0.0;

    this->modifiedClientTimer = 0;
    this->chunk = 0;

    this->isAttack = false;
    this->attackTimer = 0.0;
    this->pingLast = 0;
    this->pingTotal = 0.0;
    this->pingTimer = 0.0;
    this->rings = 0;
    this->healRings = 0;

    this->state = 0;
    this->flags = 0;
    this->deathTimerSec = 0;
    this->deathTimer = 0;
    this->revival = 0;
    this->revivalInit = { -1, -1, -1, -1, -1 };

    this->userdata.shards = 0;

    this->startPos = Vector2();
    this->position = Vector2();

    stats.reset();
}
