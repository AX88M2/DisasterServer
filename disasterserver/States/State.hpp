
#ifndef DISASTERSERVER_STATE_HPP
#define DISASTERSERVER_STATE_HPP

#include "Peer.hpp"

namespace DisasterServer
{
    template <typename T>
    class State {
    protected:
        Server *server = nullptr;
    public:
        explicit State(Server *server) {
            this->server = server;
        }

        virtual ~State() {
            server = nullptr;
        }

        virtual bool joined(Peer &peer) {
            return true;
        }

        virtual void tick() {

        }

        virtual bool handle(Peer &peer, Packet &packet) {
            return true;
        }
    };
}

#endif //DISASTERSERVER_STATE_HPP
