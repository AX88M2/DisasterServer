#include "MapVoteState.hpp"

#include "StateController.hpp"
#include "Server.hpp"

using namespace DisasterServer;

MapVoteState::MapVoteState(Server *server, StateController *controller) : State(server, controller) {
}

bool MapVoteState::joined(Client &client) {
    return true;
}

bool MapVoteState::leaved(Client &client) {
    return true;
}

bool MapVoteState::tick() {
    return true;
}

bool MapVoteState::handle(Client &client, Packet &packet) {
    switch (packet.getPacketType()) {
        case PacketType::CLIENT_VOTE_REQUEST: {
            if (client.isInGame()) {
                break;
            }

            uint8_t map = packet.read<uint8_t>();

            AssertOrDisconnect(client, !client.isVoted());
            AssertOrDisconnect(client, map >= 0);
            AssertOrDisconnect(client, map < 3);

            votes[map]++;
            client.setVoted(true);

            Packet pack(PacketType::SERVER_VOTE_SET);
            for (int i = 0; i < 3; i++) {
                pack.write<uint8_t>(votes[i]);
            }

            pack.sendBroadcast(*server);
            //mapvote_check_state(v->server);
            break;
        }

        default: break;
    }

    return true;
}
