#include "Server.hpp"

#include <algorithm>

#include "Core/Packet.hpp"
#include "Core/Log.hpp"
#include "Core/Time.hpp"

#include "StateManager.hpp"

using namespace DisasterServer;


Server::Server(const uint16_t n) : id(n), stateManager(this) {
    ENetAddress addr;
    addr.host = ENET_HOST_ANY;
    addr.port = BASE_SERVER_PORT + n;
    host = enet_host_create(&addr, 50, 2, 0, 0);

    Info("Listening on port {}", addr.port);
}

Server::~Server() {
    enet_host_destroy(host);
};

void Server::initialize() {
    TimeStamp ticker;
    time_start(&ticker);

    double next_tick = time_end(&ticker);
    double heartbeat = 0.0;
    constexpr double TARGET_FPS = 1000.0 / 60;

    //

    while (!running) {
        ENetEvent ev;
        if (enet_host_service(host, &ev, 5) > 0) {
            switch (ev.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    char ip[250];
                    enet_address_get_host_ip(&ev.peer->address, ip, 250);

                    Client *client = new Client(this, ev.peer, ev.peer->incomingPeerID + 1, ip);
                    Debug("Connection client {}", client->getId());

                    ev.peer->data = client;

                    Packet pack(PacketType::SERVER_PREIDENTITY);
                    client->getAuthPeer().one = (uint8_t) rand() % 128;
                    client->getAuthPeer().two = (uint8_t) rand() % 255;

                    uint32_t type = 0;

                    switch (rand() % 3)
                    {
                        case 0:
                            type = 1u << 9;
                            break;

                        case 1:
                            type = 1u << 31;
                            break;

                        case 2:
                            type = 1u << 26;
                            break;

                        default: break;
                    }

                    for (int bit = 0; bit < 32; ++bit)
                    {
                        if (bit == 9 || bit == 31 || bit == 26)
                            continue;

                        if (rand() % 3 <= 1)
                            type &= ~(1u << bit);
                        else
                            type |= 1u << bit;
                    }

                    client->getAuthPeer().type = type;

                    pack.write<uint16_t>(0);
                    pack.write<uint16_t>(1);
                    pack.write<uint8_t>(client->getAuthPeer().one);
                    pack.write<uint8_t>((uint8_t) rand() % 2);
                    pack.write<uint8_t>(client->getAuthPeer().two);

                    const uint8_t key[6] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };

                    for (int i = 0; i < 3; i++) {
                        pack.write<uint8_t>(key[rand() % sizeof(key)]);
                    }
                    pack.write<uint32_t>(client->getAuthPeer().type);
                    pack.send(*client, true);

                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    Client *client = static_cast<Client*>(ev.peer->data);

                    if (!client) {
                        break;
                    }

                    if (client->isOpped() && client->isShouldTimeout()) {

                    }

                    if (client->isVerified()) {
                        auto it = std::find_if(
                            peers.begin(),
                            peers.end(),
                            [client](const auto& p) {
                                return p == client;
                            });

                        if (it != peers.end()) {
                            stateManager.state_left(**it);
                            peers.erase(it);
                        }
                    }

                    Info("{} (id {}) left.", client->getNickname(), client->getId());
                    delete client;
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    Client *client = static_cast<Client*>(ev.peer->data);
                    Packet packet(ev.packet);

                    switch (packet.getPacketType()) {
                        case PacketType::IDENTITY: {
                            if (!client->identity(packet)) {
                                Debug("Identity failed for id {}", client->getId());
                            }
                            break;
                        }
                        default: {
                            if (!client->message_received(packet)) {
                                break;
                            }
                            break;
                        }
                    }

                    break;
                }

                default: break;
            }
        }

        double now = time_end(&ticker);
        while (next_tick < now) {
            next_tick += TARGET_FPS;

            stateManager.state_tick();

            // Heartbeat
            if (peers.empty()) {
                Packet packet_heartbeat(PacketType::SERVER_HEARTBEAT);
                if (heartbeat >= (TICKSPERSEC * 2))
                {
                    broadcast(packet_heartbeat, true);
                    Debug("Heartbeat done.");
                    heartbeat = 0;
                }
                heartbeat += delta;
            }

            delta = 1;
        }
    }
}

void Server::disconnect_by_id(const uint16_t id, DisconnectReason reason, const std::string &message) {
    for (auto client : peers) {
        if (client->getId() == id) {
            return client->disconnect(reason, message);
        }
    }
}

void Server::broadcast(Packet &packet, bool reliable) {
    Debug("PacketType::{} sending broadcast", getPacketTypeName(packet.getPacketType()));

    for (auto client : peers) {
        packet.send(*client, reliable);
    }
}

void Server::broadcast_ex(Packet &packet, bool reliable, uint16_t ignore) {
    Debug("PacketType::{} sending broadcast, ignoring client {}", getPacketTypeName(packet.getPacketType()), ignore);
    for (auto client : peers) {
        if (client->getId() == ignore) {
            continue;
        }
        packet.send(*client, reliable);
    }
}

void Server::send_message(Client &client, std::string message) {
    std::ranges::transform(message, message.begin(), [](const unsigned char c){ return std::tolower(c); });
    Packet packet(PacketType::CLIENT_CHAT_MESSAGE);
    packet.write<uint16_t>(0);
    packet.writeString(message);
    packet.send(client, true);
}

void Server::send_broadcast_message(uint16_t sender, std::string &message) {
    Packet packet(PacketType::CLIENT_CHAT_MESSAGE);

    packet.write<uint16_t>(sender);
    packet.writeString(message);

    broadcast(packet, true);
}

StateManager &Server::getStateManager() {
    return stateManager;
}