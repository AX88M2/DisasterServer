#include "Packet.hpp"

#include "Log.hpp"

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

#undef min

Packet::Packet(const enet_uint8 *data, size_t data_size) {
	std::copy_n(data, std::min(data_size, buffer.size()), buffer.begin());
	len = data_size;
	type = static_cast<PacketType>(255);
}

Packet::Packet(PacketType type) : buffer({}), type(type) {
	Debug("Creating packet PacketType::{}", getPacketTypeName(type));
	writeUint8(0);
	writeUint8(static_cast<uint8_t>(type));
}

Packet::~Packet() = default;

std::string Packet::readString() {
	std::string result;

	while (position < buffer.size()) {
		char ch = static_cast<char>(buffer[position++]);

		if (ch == '\0')
			break;

		result += ch;
	}

	return result;
}

void Packet::writeUint8(uint8_t value) {
	if (position + sizeof(uint8_t) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(uint8_t) >= len) {
		len++;
	}

	buffer[position++] = value;
}

void Packet::writeUint16(uint16_t value) {
	if (position + sizeof(uint16_t) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(uint16_t) >= len) {
		len += sizeof(uint16_t);
	}

	uint8_t *ptr = (uint8_t*)&value;
	buffer[position++] = ptr[0];
	buffer[position++] = ptr[1];
}

void Packet::writeUint32(uint32_t value) {
	if (position + sizeof(uint32_t) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(uint32_t) >= len) {
		len += sizeof(uint32_t);
	}

	uint8_t *ptr = (uint8_t*)&value;
	buffer[position++] = ptr[0];
	buffer[position++] = ptr[1];
	buffer[position++] = ptr[2];
	buffer[position++] = ptr[3];
}

void Packet::writeUint64(uint64_t value) {
	if (position + sizeof(uint64_t) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(uint64_t) >= len) {
		len += sizeof(uint64_t);
	}

	uint8_t *ptr = (uint8_t*)&value;
	buffer[position++] = ptr[0];
	buffer[position++] = ptr[1];
	buffer[position++] = ptr[2];
	buffer[position++] = ptr[3];
	buffer[position++] = ptr[4];
	buffer[position++] = ptr[5];
	buffer[position++] = ptr[6];
	buffer[position++] = ptr[7];
}

void Packet::writeFloat(float value) {
	if (position + sizeof(float) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(float) >= len) {
		len += sizeof(float);
	}

	uint8_t *ptr = (uint8_t*)&value;
	buffer[position++] = ptr[0];
	buffer[position++] = ptr[1];
	buffer[position++] = ptr[2];
	buffer[position++] = ptr[3];
}

void Packet::writeDouble(double value) {
	if (position + sizeof(double) > PACKET_MAXSIZE) {
		Err("Exceeding the Packet Size Limit. Max Size {}", PACKET_MAXSIZE);
		return;
	}

	if (position + sizeof(double) >= len) {
		len += sizeof(double);
	}

	uint8_t *ptr = (uint8_t*)&value;
	buffer[position++] = ptr[0];
	buffer[position++] = ptr[1];
	buffer[position++] = ptr[2];
	buffer[position++] = ptr[3];
	buffer[position++] = ptr[4];
	buffer[position++] = ptr[5];
	buffer[position++] = ptr[6];
	buffer[position++] = ptr[7];
}

void Packet::writeString(const std::string &value) {
	for (uint8_t i = 0; i < value.size(); i++) {
		writeUint8(value[i]);
	}
}

void Packet::send(enetpp::server<Peer> &server, uint32_t client_id, bool reliable) {
	Debug("packet PacketType::{} sending to {}", getPacketTypeName(type), client_id);
	server.send_packet_to(client_id, reliable ? 0 : 1, buffer.data(), len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
}
