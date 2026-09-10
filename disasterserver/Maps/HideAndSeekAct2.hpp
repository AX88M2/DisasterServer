#ifndef DISASTERSERVER_HIDEANDSEEKACT2_HPP
#define DISASTERSERVER_HIDEANDSEEKACT2_HPP

#include "Core/Map.hpp"

namespace DisasterServer::Maps
{
    class HideAndSeekAct2 : public Map {
    public:
        HideAndSeekAct2(Server *server);

        void init() override;

        void tick() override;

        void handle(Client &client, Packet &packet) override;

        void left(Client &client) override;
    };
}

#endif //DISASTERSERVER_HIDEANDSEEKACT2_HPP
