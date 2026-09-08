#include <Auth.h>
#include <Server.h>
#include <stdbool.h>
#include <stdint.h>

bool auth_create_ticket(PeerData* peer, Packet* packet)
{
	peer->auth.one = (uint8_t) rand() % 128;
	peer->auth.two = (uint8_t) rand() % 255;

	uint32_t type = 0;

	switch (rand() % 3)
	{
		case 0:
			type = 1u << 9;
			break;

		case 1:
			type = 1u << 31;
			break;

		case 2:
			type = 1u << 26;
			break;

		default: break;
	}

	for (int bit = 0; bit < 32; ++bit)
	{
		if (bit == 9 || bit == 31 || bit == 26)
			continue;

		if (rand() % 3 <= 1)
			type &= ~(1u << bit);
		else
			type |= 1u << bit;
	}


	peer->auth.type = type;
	// Red herrings? Nah, we have Red Rope(TM)
	// All values here are complete bogus btw
	PacketWrite(packet, packet_write16, 0);
	PacketWrite(packet, packet_write16, 1);
	PacketWrite(packet, packet_write8, peer->auth.one);
	PacketWrite(packet, packet_write8, (uint8_t) rand() % 2);
	PacketWrite(packet, packet_write8, peer->auth.two);
	for (int i = 0; i < 3; i++)
	{
		const char key[] = { 0x00, 0x00, 0xFF, 0x1F, 0x80, 0x14 };
		PacketWrite(packet, packet_write8, key[rand() % sizeof(key)]);
	}
	PacketWrite(packet, packet_write32, peer->auth.type);
	return true;
}

bool auth_verify_ticket(PeerData* v, Packet* packet)
{
	PacketRead(checkcum, packet, packet_read64, uint64_t);
	PacketRead(checkcum2, packet, packet_read64, uint64_t);

	Debug("checkcum : %x", checkcum);
	Debug("checkcum2 : %x", checkcum2);

	uint64_t check = checkcum - v->auth.type;
	uint64_t checka = checkcum2 - v->auth.type;

	Debug("check : %x", check);
	Debug("checka : %x", checka);

	if ((v->auth.type >> 9) & 1)
	{
		if (check != 0x2f09cdda)
			v->mod_tool = 1;

		if (checka != 0xf1006056)
			v->mod_tool = 1;
	}
	else if ((v->auth.type & 0x80000000) != 0)
	{
		if (check != 0x947)
			v->mod_tool = 1;

		if (checka != 0xb43)
			v->mod_tool = 1;
	}
	else if ((v->auth.type >> 26) & 1)
	{
		if (check != 0xdcd)
			v->mod_tool = 1;

		if (checka != 0xc15)
			v->mod_tool = 1;
	}
	else
	{
		v->mod_tool = 1;
	}
	return true;
}
