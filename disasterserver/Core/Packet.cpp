#include "Packet.hpp"

#include <utility>

#include "Log.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Exceptions.hpp"

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

Packet::Packet(ENetPacket *packet) : buffer({}) {
	if (!packet) {
		throw std::invalid_argument("Packet is null");
	}

	if (packet->dataLength > buffer.size()) {
		enet_packet_destroy(packet);
		throw PacketError::Format("Packet is too large");
	}

	len = std::min(static_cast<size_t>(packet->dataLength), buffer.size());

	std::memcpy(buffer.data(), packet->data, len);
	enet_packet_destroy(packet);

	if (len < 2) {
		throw std::runtime_error("Packet is too small");
	}

	read<uint8_t>();
	const auto rawType = read<uint8_t>();

	type = static_cast<PacketType>(rawType);
}

Packet::Packet(PacketType type) : buffer({}), type(type) {
	write<uint8_t>(0);
	write<uint8_t>(static_cast<uint8_t>(type));
}

Packet::~Packet() = default;

std::string Packet::readString() {
	std::string result;

	while (position < len) {
		const char c = static_cast<char>(buffer[position++]);

		if (c == '\0')
			return result;

		result += c;
	}

	throw PacketError::Format("Unterminated string");
}

void Packet::writeString(const std::string &value) {
	if (position > PACKET_MAXSIZE || value.size() > PACKET_MAXSIZE - position - 1) {
		throw std::runtime_error("String is too long");
	}

	for (unsigned char c : value) {
		write<uint8_t>(c);
	}

	write<uint8_t>(0);
}

bool Packet::send(Client &client, bool reliable) {
	if(client.isDisconnecting())
		return true;

	Debug("PacketType::{} sending to {}", getPacketTypeName(type), client.getId());

	ENetPacket* pack = enet_packet_create(buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	if (!pack) {
		Err("Failed to create ENet packet");
		return false;
	}

	const int result = enet_peer_send(
		client.getPeer(),
		reliable ? 0 : 1,
		pack
	);

	if (result != 0) {
		return false;
	}

	return true;
}

void Packet::sendBroadcast(Server &server, bool reliable, std::function<bool(const Client& client)> predicate) {
	for (auto &client : server.getPeers()) {
		if (predicate(*client)) {
			if (!send(*client, reliable)) {
				Warn("Failed to send {} to client {}", getPacketTypeName(type), client->getId());
			}
		}
	}
}