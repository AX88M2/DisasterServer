using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class CreamRing : Ring
{
	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		IID = byte.MaxValue;
		ID = map.RingIDs++;
		return new TcpPacket(PacketType.SERVER_RING_STATE, (byte)2, (ushort)X, (ushort)Y, IID, ID, IsRedRing);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_RING_STATE, (byte)1, IID, ID);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		return null;
	}
}
