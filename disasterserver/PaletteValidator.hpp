#pragma once

#include <cstdint>

namespace DisasterServer
{

    class Client;
    class Packet;

    bool validatePlayerPalette(const Client& client, Packet& packet);

} // namespace DisasterServer