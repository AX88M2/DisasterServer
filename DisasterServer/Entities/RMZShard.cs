using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class RMZShard : Entity
{
	private static byte GID;

	public byte ID = GID++;

	private bool _isSpawned;

	public RMZShard(int x, int y, bool spawned = false)
	{
		X = x;
		Y = y;
		_isSpawned = spawned;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_RMZSHARD_STATE, (byte)(_isSpawned ? 1u : 0u), ID, (ushort)X, (ushort)Y);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		return null;
	}
}
