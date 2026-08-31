#include "Server.hpp"

#include "Log.hpp"

Server::Server(uint16_t basePort, uint16_t id) : id(id) {
    for (int i = 0; i < 20; i++) {
        map_pickrates[i] = 255;
    }

    ENetAddress addr;
    addr.host = ENET_HOST_ANY;
    addr.port = basePort + id;
    host = enet_host_create(&addr, 50, 2, 0, 0);

    Info("Listening on port {:d}.", basePort + id);

    RAssertEx(host != nullptr);
}

Server::~Server() = default;

void Server::worker() {
    srand (time(nullptr));

    TimeStamp ticker;
    time_start(&ticker);

    double next_tick = time_end(&ticker);
    double heartbeat = 0.0;
    const double TARGET_FPS = 1000.0 / 60;

    Packet packet(PacketType::SERVER_HEARTBEAT);
    while (running) {
        ENetEvent ev;
        if (enet_host_service(host, &ev, 5) > 0) {
            switch (ev.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    Debug("ENET_EVENT_TYPE_CONNECT...");
                    ev.peer->data = static_cast<PeerData *>(malloc(sizeof(PeerData)));
                    if (!ev.peer->data)
                        return;

                    memset(ev.peer->data, 0, sizeof(PeerData));

                    auto *v = static_cast<PeerData *>(ev.peer->data);
                    v->server = this;
                    v->peer = ev.peer;
                    v->id = ev.peer->incomingPeerID + 1;

                    char buffer[0x100];
                    enet_address_get_host_ip(&ev.peer->address, buffer, 250);
                    v->ip = std::string(buffer);

                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    Debug("ENET_EVENT_TYPE_DISCONNECT...");
                    PeerData *v = (PeerData *)ev.peer->data;
                    if (!v)
                        break;

                    /*if (!v->op && v->should_timeout)
                    {
                        uint64_t result;
                        if (timeout_check(v->udid.value, v->ip.value, &result) && result == 0)
                            timeout_set(v->nickname.value, v->udid.value, v->ip.value, time(NULL) + 5);
                    }

                    if (v->verified)
                    {
                        MutexLock(ip_addr_mut);
                        {
                            cJSON_DeleteItemFromObject(ip_addr_list, v->udid.value);
                            cJSON_DeleteItemFromObject(ip_addr_list, v->ip.value);
                        }
                        MutexUnlock(ip_addr_mut);

                        MutexLock(v->server->state_lock);
                        {
                            // Step 3: Cleanup (Only if joined before)
                            if (dylist_remove(&v->server->peers, v))
                                server_state_left(v);
                        }
                        MutexUnlock(v->server->state_lock);
                    }*/

                    Info("{} (id {:d}) " LOG_YLW "left.", v->nickname.value, v->id);
                    free(v);
                    break;
                }

                default: break;
            }
        }

        double now = time_end(&ticker);
        while (next_tick < now) {
            next_tick += TARGET_FPS;
            mutex.lock();
            {
                switch (state) {
                    case States::LOBBY:
                    case States::CHARSELECT:
                    case States::MAPVOTE:
                        break;

                    case States::GAME:
                        break;

                    case States::RESULTS:
                        break;

                    default: break;
                }

                if (peers.empty()) {
                    if (heartbeat >= (TICKSPERSEC * 2))
                    {
                        Debug("Heartbeat done.");
                        heartbeat = 0;
                    }
                    heartbeat += delta;
                }
            }
            mutex.unlock();
            delta = 1;
        }
    }

    enet_host_destroy(host);
}
