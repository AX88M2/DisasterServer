#include "Packet.hpp"

#include "Core/Log.hpp"
#include "Core/Exceptions.hpp"
#include "Client.hpp"
#include "Server.hpp"

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

	[[maybe_unused]]
	const uint8_t isPasstrough = read<uint8_t>();
	type = read<PacketType>();
	Debug("Packet received {} (length {})", getPacketTypeName(type), len);
}

Packet::Packet(PacketType type) : buffer({}), type(type) {
	write<uint8_t>(0);
	write<PacketType>(type);
	if (type != PacketType::SERVER_HEARTBEAT) {
		Debug("Packet created {}", getPacketTypeName(type));
	}
}

Packet::~Packet() = default;

void Packet::seek(const size_t offset) {
	if (offset > this->len) {
		throw PacketError::format("Invalid packet offset");
	}

	const size_t amount = this->len - offset;

	if (amount > buffer.size() - position) {
		Error("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		throw PacketError::format("Packet overflow");
	}

	position = offset;
}

void Packet::append(const Packet &other, size_t offset) {
	if (offset > other.len) {
		throw PacketError::format("Invalid packet offset");
	}

	const size_t amount = other.len - offset;

	if (amount > buffer.size() - position) {
		Error("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		throw PacketError::format("Packet overflow");
	}

	std::memcpy(buffer.data() + position, other.buffer.data() + offset, amount);

	position += amount;
	len = std::max(len, position);
}

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

Vector2 Packet::readVector2() {
	const float x = read<uint16_t>();
	const float y = read<uint16_t>();

	return { static_cast<float>(x), static_cast<float>(y) };
}

void Packet::writeVector2(const Vector2 &value) {
	write<uint16_t>(static_cast<uint16_t>(value.x));
	write<uint16_t>(static_cast<uint16_t>(value.y));
}

bool Packet::send(Client &client, bool reliable) {
	if(client.isDisconnecting()) {
		return true;
	}

	Debug("{} sending to {} (id {})", getPacketTypeName(type), client.getNickname(), client.getId());

	ENetPacket* pack = enet_packet_create(buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	if (!pack) {
		Error("Failed to create ENet packet");
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