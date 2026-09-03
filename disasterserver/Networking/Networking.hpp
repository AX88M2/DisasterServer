#ifndef DISASTERSERVER_NETWORKING_HPP
#define DISASTERSERVER_NETWORKING_HPP

#include <format>
#include <functional>
#include <stdexcept>

#include "enet/enet.h"

namespace DisasterServer::Networking
{
    class NetworkingException : public std::exception {
    public:
        explicit NetworkingException(const std::string& message);

        template <typename ...Args>
        static NetworkingException Format(std::string_view fmt, Args&&... args) {
            return NetworkingException(std::vformat(fmt, std::make_format_args(args...)));
        }
    };

    class NetworkingInitializationException : public NetworkingException {
    public:
        explicit NetworkingInitializationException(const std::string& message);

        template <typename ...Args>
        static NetworkingInitializationException Format(std::string_view fmt, Args&&... args) {
            return NetworkingInitializationException(std::vformat(fmt, std::make_format_args(args...)));
        }
    };

    enum class NetworkingEventType {
        NONE,
        CONNECT_EVENT,
        DISCONNECT_EVENT,
        RECEIVE_EVENT,
    };

    void initialize() {
        if (enet_initialize() != 0) {
            throw NetworkingInitializationException::Format("Failed to initialize enet.");
        }
    };

    class entrypoint {
        ENetAddress _address = {};
    public:
        entrypoint(uint16_t port) {
            _address.host = ENET_HOST_ANY;
            _address.port = port;
        }

        ENetAddress &getAddress() {
            return _address;
        }
    };

    class ENetSocket {
        ENetHost* host = nullptr;
        ENetEvent ev = {};
    public:
        ENetSocket(entrypoint ep) {
            this->host = enet_host_create(&ep.getAddress(), 50, 2, 0, 0);
        }

        void pullEvents(uint32_t timeout, std::function<void(ENetEventType, ENetPeer*)> callback) {
            if (enet_host_service(host, &ev, timeout) > 0) {
                callback(ev.type, ev.peer);
            }
        }

        ~ENetSocket() {
            if (host != nullptr) {
                enet_host_destroy(host);
            }
        }
    };
}

#endif //DISASTERSERVER_NETWORKING_HPP
