#include "Acid.hpp"

#include "Server.hpp"
#include "Core/Constansts.hpp"

using namespace DisasterServer;
using namespace DisasterServer::Entities;

Acid::Acid(entityId id, Server &server, GameState &state, const Vector2 &position) : Entity(id, server, state, "acid", position) {
}

Acid::~Acid() = default;

bool Acid::init() {
    return Entity::init();
}

bool Acid::tick() {
    if (timer >= 4 * TICKS_PER_SEC) {
        timer = 0;
        activated = !activated;
        if (activated) {
            acid_id = server.getRandom().nextInt(0, 7);
        }

        Packet pack(PacketType::SERVER_TCGOM_STATE);
        pack.write<uint8_t>(acid_id);
        pack.write<uint8_t>(activated);
        pack.sendBroadcast(server, true);
    }

    timer += server.getDelta();
    return Entity::tick();
}

bool Acid::uninit() {
    return true;
}
