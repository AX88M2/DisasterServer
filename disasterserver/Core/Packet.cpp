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

#define READ_TYPE(type) \
	type value = reinterpret_cast<type*>(buffer.data() + position)[0]; \
	position += sizeof(type); \

Packet::Packet(const std::array<uint8_t, PACKET_MAXSIZE> &buffer) : buffer(buffer) {

}

Packet::Packet(PacketType type) : buffer({}) {
	writeUint8(0);
	writeUint8(static_cast<uint8_t>(type));
}

Packet::~Packet() = default;

std::optional<uint8_t> Packet::readUint8() {
	READ_TYPE(uint8_t);
	return value;
}

std::optional<uint16_t> Packet::readUint16() {
	READ_TYPE(uint16_t);
	return value;
}

std::optional<uint32_t> Packet::readUint32() {
	READ_TYPE(uint32_t);
	return value;
}

std::optional<uint64_t> Packet::readUint64() {
	READ_TYPE(uint64_t);
	return value;
}

std::optional<float> Packet::readFloat() {
	READ_TYPE(float);
	return value;
}

std::optional<double> Packet::readDouble() {
	READ_TYPE(double);
	return value;
}

std::optional<std::string> Packet::readString() {
	char buf[250];

	while (true) {
		char ch = buffer[position];
		buf[position] = ch;
		position++;

		if (ch == '\0')
			break;
	}

	return std::string(buf);
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

void Packet::send(udp::endpoint &endpoint, udp::socket &socket) {
	socket.send_to(boost::asio::buffer(buffer, len), endpoint);
}
