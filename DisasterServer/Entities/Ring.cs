using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class Ring : Entity
{
	public ushort ID = 1;

	public byte IID;

	public bool IsRedRing;

	private static readonly Random _rand = new Random();

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		ID = map.RingIDs++;
		IsRedRing = map.CanSpawnRedRings() && _rand.Next(100) <= 10;
		return new TcpPacket(PacketType.SERVER_RING_STATE, (byte)0, IID, ID, IsRedRing);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		map.FreeRingID(IID);
		return new TcpPacket(PacketType.SERVER_RING_STATE, (byte)1, IID, ID);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		return null;
	}
}
