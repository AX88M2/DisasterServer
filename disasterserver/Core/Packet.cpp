#include "Packet.hpp"

#include <utility>

#include "Log.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "Exceptions.hpp"



using namespace DisasterServer;

Packet::Packet(ENetPacket *packet) : buffer({}) {
	if (!packet) {
		throw PacketError::format("Packet is null");
	}

	if (packet->dataLength > buffer.size()) {
		enet_packet_destroy(packet);
		throw PacketError::format("Packet is too large {} bytes", packet->dataLength);
	}

	len = packet->dataLength;

	std::memcpy(buffer.data(), packet->data, len);
	enet_packet_destroy(packet);

	if (len < 2) {
		throw PacketError::format("Packet is too small");
	}

	read<uint8_t>();
	type = static_cast<PacketType>(read<uint8_t>());

	Debug("Packet received {}", getPacketTypeName(type));
}

Packet::Packet(PacketType type) : buffer({}), type(type) {
	write<uint8_t>(0);
	write<uint8_t>(static_cast<uint8_t>(type));
	if (type != PacketType::SERVER_HEARTBEAT) {
		Debug("Packet created {}", getPacketTypeName(type));
	}
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

	throw PacketError::format("Unterminated string");
}

void Packet::writeString(const std::string &value) {
	if (position >= buffer.size() || value.size() >= buffer.size() - position) {
		throw PacketError::format("String is too long: {} bytes", value.size());
	}

	for (unsigned char c : value) {
		write<uint8_t>(c);
	}

	write<uint8_t>(0);
}

bool Packet::send(Client &client, bool reliable) {
	if(client.isDisconnecting())
		return true;

	Debug("{} sending to {} (id {})", getPacketTypeName(type), client.getNickname(), client.getId());

	ENetPacket* pack = enet_packet_create(buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	if (!pack) {
		Err("Failed to create ENet packet");
		return false;
	}

	return enet_peer_send(client.getPeer(), reliable ? 0 : 1, pack) == 0;
}

void Packet::sendBroadcast(Server &server, bool reliable, std::function<bool(const Client& client)> predicate) {
	for (auto &client : server.getClients()) {
		if (predicate(*client)) {
			if (!send(*client, reliable)) {
				Warn("Failed to send {} to client {}", getPacketTypeName(type), client->getId());
			}
		}
	}
}