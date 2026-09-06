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
                    char buf[250];
                    enet_address_get_host_ip(&ev.peer->address, buf, 250);

                    const uint16_t id = ev.peer->incomingPeerID + 1;

                    auto client = std::make_unique<Client>(this, ev.peer, id, buf);
                    Client *rawClient = client.get();
                    ev.peer->data = rawClient;
                    peers.push_back(std::move(client));

                    Debug("Connection client {}", rawClient->getId());

                    Packet pack(PacketType::SERVER_PREIDENTITY);

                    auto &auth = rawClient->getAuthPeer();
                    auth.one = (uint8_t) rand() % 128;
                    auth.two = (uint8_t) rand() % 255;

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

                    auth.type = type;

                    pack.write<uint16_t>(0);
                    pack.write<uint16_t>(1);
                    pack.write<uint8_t>(auth.one);
                    pack.write<uint8_t>((uint8_t) rand() % 2);
                    pack.write<uint8_t>(auth.two);

                    const uint8_t key[6] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };

                    for (int i = 0; i < 3; i++) {
                        pack.write<uint8_t>(key[rand() % sizeof(key)]);
                    }

                    pack.write<uint32_t>(auth.type);
                    pack.send(*rawClient, true);

                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    Client *client = static_cast<Client*>(ev.peer->data);

                    if (!client) {
                        break;
                    }

                    Info("{} (id {}) left.", client->getNickname(), client->getId());

                    if (client->isOpped() && client->isShouldTimeout()) {

                    }

                    if (client->isVerified()) {
                        stateManager.state_left(*client);
                    }

                    ev.peer->data = nullptr;

                    auto it = std::find_if(
                            peers.begin(),
                            peers.end(),
                            [client](const auto& p) {
                                return p.get() == client;
                            });

                    if (it != peers.end()) {
                        peers.erase(it);
                    }

                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    Client *client = static_cast<Client*>(ev.peer->data);

                    if (!client) {
                        enet_packet_destroy(ev.packet);
                        break;
                    }

                    switch (static_cast<PacketType>(ev.packet->data[1])) {
                        case PacketType::IDENTITY: {
                            Packet packet(ev.packet);
                            if (!client->identity(packet)) {
                                Debug("Identity failed for id {}", client->getId());
                            }
                            break;
                        }
                        default: {
                            Packet packet(ev.packet);
                            client->message_received(packet);
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
                Packet pack(PacketType::SERVER_HEARTBEAT);
                if (heartbeat >= (TICKSPERSEC * 2))
                {
                    pack.sendBroadcast(*this, true);
                    Debug("Heartbeat done.");
                    heartbeat = 0;
                }
                heartbeat += delta;
            }

            delta = 1;
        }
    }
}

void Server::disconnect_by_id(const uint16_t client_id, DisconnectReason reason, const std::string &message) {
    for (auto &client : peers) {
        if (client->getId() == client_id) {
            return client->disconnect(reason, message);
        }
    }
}

void Server::broadcast_ex(Packet &packet, bool reliable, uint16_t ignore) {
    Debug("PacketType::{} sending broadcast, ignoring client {}", getPacketTypeName(packet.getPacketType()), ignore);
    packet.sendBroadcast(*this, reliable, [ignore](const Client& v) { return v.getId() == ignore; });
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
    packet.sendBroadcast(*this, true);
}

StateManager &Server::getStateManager() {
    return stateManager;
}