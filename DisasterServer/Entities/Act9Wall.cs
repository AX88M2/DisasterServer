using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class Act9Wall : Entity
{
	private byte _id;

	private ushort _tx;

	private ushort _ty;

	private int _startTime;

	public Act9Wall(byte id, ushort tx, ushort ty)
	{
		_id = id;
		_tx = tx;
		_ty = ty;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		_startTime = map.Timer;
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		X = (int)((double)(int)_tx * ((double)(_startTime - map.Timer) / (double)_startTime));
		Y = (int)((double)(int)_ty * ((double)(_startTime - map.Timer) / (double)_startTime));
		return new UdpPacket(PacketType.SERVER_ACT9WALL_STATE, _id, (ushort)X, (ushort)Y);
	}
}
