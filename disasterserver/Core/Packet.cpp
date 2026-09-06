#include "Packet.hpp"

#include <utility>

#include "Log.hpp"
#include "Client.hpp"
#include "Server.hpp"

#ifdef __GNUC__ // GCC, clang...
	#define BYTESWAP_16(x) __builtin_bswap16((x))
	#define BYTESWAP_32(x) __builtin_bswap32((x))
	#define BYTESWAP_64(x) __builtin_bswap64((x))
#else
	#define BYTESWAP_16(x) _byteswap_ushort((x))
	#define BYTESWAP_32(x) _byteswap_ulong((x))
	#define BYTESWAP_64(x) _byteswap_uint64((x))
#endif

using namespace DisasterServer;

Packet::Packet(const std::array<uint8_t, PACKET_MAXSIZE> &buffer) : buffer(buffer) {
	read<uint8_t>();
	type = static_cast<PacketType>(read<uint8_t>());
}

Packet::Packet(ENetPacket *packet) : buffer({}) {
	std::copy_n(packet->data, std::min(packet->dataLength, buffer.size()), buffer.begin());
	enet_packet_destroy(packet);
	read<uint8_t>();
	type = static_cast<PacketType>(read<uint8_t>());
}

Packet::Packet(PacketType type) : buffer({}), type(type) {
	write<uint8_t>(0);
	write<uint8_t>(static_cast<uint8_t>(type));
}

Packet::~Packet() = default;

std::string Packet::readString() {
	std::string result;

	while (position < buffer.size()) {
		auto ch = static_cast<char32_t>(buffer[position++]);

		if (ch == '\0')
			break;

		result += ch;
	}

	return result;
}

void Packet::writeString(const std::string &value) {
	for (char i : value) {
		write<uint8_t>(i);
	}
}

bool Packet::send(Client &client, bool reliable) {
	Debug("PacketType::{} sending to {} (id {})", getPacketTypeName(type), client.getNickname(), client.getId());

	ENetPacket* pack = enet_packet_create(buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	return enet_peer_send(client.getPeer(), reliable ? 0 : 1, pack);
}

void Packet::sendBroadcast(Server &server, bool reliable, std::function<bool(const Client& client)> predicate) {
	ENetPacket* pack = enet_packet_create(buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	for (auto client : server.getPeers()) {
		if (predicate(*client)) {
			enet_peer_send(client->getPeer(), reliable ? 0 : 1, pack);
		}
	}
}