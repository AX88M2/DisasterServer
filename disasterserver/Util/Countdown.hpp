#ifndef DISASTERSERVER_COUNTDOWN_HPP
#define DISASTERSERVER_COUNTDOWN_HPP

#include <functional>

#include "Core/Types.hpp"

namespace DisasterServer
{
    class Server;

    class Countdown {
        Server *server = nullptr;

        double countdown = 0;
        uint8_t countdownSec = 0;

        std::function<void()> endOfCountdown = [] {};

    public:
        Countdown(Server *server);

        void start(uint8_t seconds);
        void update();

        void setEndOfCountdown(const std::function<void()> callback) { this->endOfCountdown = callback; }

        uint8_t getCountdownSec() { return countdownSec; }
    };
}

#endif //DISASTERSERVER_COUNTDOWN_HPP
