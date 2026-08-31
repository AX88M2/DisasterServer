#include "Packet.hpp"
#include "Server.hpp"
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

Packet::Packet() {
}

Packet::Packet(PacketType type) {
	write8(0);
	write8(static_cast<uint8_t>(type));
}

Packet::Packet(ENetPacket *packet) {
	memcpy(buffer, packet->data, (uint8_t)packet->dataLength);
	enet_packet_destroy(packet);
}

bool Packet::seek(int wh) {
	RAssert(wh >= 0);
	RAssert(wh < PACKET_MAXSIZE);
	RAssert(wh < len);

	pos = wh;
	return true;
}

bool Packet::read8(uint8_t *out) {
	RAssert(pos < len);
	*out = buffer[pos++];

	return true;
}

bool Packet::read16(uint16_t *out) {
	RAssert(pos < len);
	*out = *((int16_t*)&buffer[pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_16(*out);
#endif

	pos += 2;

	return true;
}

bool Packet::read32(uint32_t *out) {
	RAssert(pos < len);
	*out = *((int32_t*)&buffer[pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_32(*out);
#endif

	pos += 4;

	return true;
}

bool Packet::read64(uint64_t *out) {
	RAssert(pos < len);
	*out = *((int64_t*)&buffer[pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_64(*out);
#endif

	pos += 8;

	return true;
}

bool Packet::readFloat(float *out) {
	RAssert(pos < len);
	*out = *((float*)&buffer[pos]);

#ifdef SYS_BIG_ENDIAN
	*out = BYTESWAP_32(*out);
#endif

	pos += 4;

	return true;
}

bool Packet::readDouble(double *out) {
	RAssert(pos < len);
	*out = *((float*)&buffer[pos]);

#ifdef SYS_BIG_ENDIAN
	* out = BYTESWAP_32(*out);
#endif

	pos += 8;

	return true;
}

bool Packet::readStr(String *out) {
	out->len = 0;

	while (true)
	{
		RAssert(pos < len);
		RAssert(out->len < 128);

		char ch = buffer[pos];
		out->value[out->len] = ch;

		pos++;
		out->len++;

		if (ch == '\0')
			break;
	}

	return true;
}

bool Packet::write8(uint8_t value) {
	RAssert(pos + 1 < PACKET_MAXSIZE);

	if (pos + 1 >= len)
		len++;

	buffer[pos++] = value;
	return true;
}

bool Packet::write16(uint16_t value) {
	RAssert(pos + 2 < PACKET_MAXSIZE);

	if (pos + 2 >= len)
		len += 2;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_16(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	buffer[pos++] = ptr[0];
	buffer[pos++] = ptr[1];
	return true;
}

bool Packet::write32(uint32_t value) {
	RAssert(pos + 4 < PACKET_MAXSIZE);

	if (pos + 4 >= len)
		len += 4;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_32(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	buffer[pos++] = ptr[0];
	buffer[pos++] = ptr[1];
	buffer[pos++] = ptr[2];
	buffer[pos++] = ptr[3];
	return true;
}

bool Packet::write64(uint64_t value) {
	RAssert(pos + 8 < PACKET_MAXSIZE);

	if (pos + 8 >= len)
		len += 8;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_64(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	buffer[pos++] = ptr[0];
	buffer[pos++] = ptr[1];
	buffer[pos++] = ptr[2];
	buffer[pos++] = ptr[3];
	buffer[pos++] = ptr[4];
	buffer[pos++] = ptr[5];
	buffer[pos++] = ptr[6];
	buffer[pos++] = ptr[7];
	return true;
}

bool Packet::writeFloat(float value) {
	RAssert(pos + 4 < PACKET_MAXSIZE);

	if (pos + 4 >= len)
		len += 4;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_32(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	buffer[pos++] = ptr[0];
	buffer[pos++] = ptr[1];
	buffer[pos++] = ptr[2];
	buffer[pos++] = ptr[3];
	return true;
}

bool Packet::writeDouble(double value) {
	RAssert(pos + 8 < PACKET_MAXSIZE);

	if (pos + 8 >= len)
		len += 8;

#ifdef SYS_BIG_ENDIAN
	value = BYTESWAP_64(value);
#endif

	uint8_t* ptr = (uint8_t*)&value;
	buffer[pos++] = ptr[0];
	buffer[pos++] = ptr[1];
	buffer[pos++] = ptr[2];
	buffer[pos++] = ptr[3];
	buffer[pos++] = ptr[4];
	buffer[pos++] = ptr[5];
	buffer[pos++] = ptr[6];
	buffer[pos++] = ptr[7];
	return true;
}

bool Packet::writeStr(String value) {
	for (uint8_t i = 0; i < value.len; i++)
		RAssert(write8(value.value[i]));

	return true;
}

bool Packet::send(ENetPeer *peer, bool reliable) {
	PeerData* data = static_cast<PeerData *>(peer->data);
	if(data && data->disconnecting)
		return true;

	pos = 0;
	ENetPacket* pack = enet_packet_create(this, len, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
	return enet_peer_send(peer, reliable ? 0 : 1, pack) == 0;
}
