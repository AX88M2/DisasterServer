#include "Countdown.hpp"
#include "Server.hpp"

using namespace DisasterServer;

Countdown::Countdown(Server *server) : server(server){
}

void Countdown::start(uint8_t seconds) {
    countdownSec = seconds;
    countdown = TICKSPERSEC;
}

void Countdown::update() {
    if (countdown <= 0) {
        countdown += TICKSPERSEC;

        if (--countdownSec == 0) {
            endOfCountdown();
            return;
        }

        Packet pack(PacketType::SERVER_CHAR_TIME_SYNC);
        pack.write<uint8_t>(countdownSec);
        pack.sendBroadcast(*server, true);
    }

    countdown -= server->getDelta();
}
