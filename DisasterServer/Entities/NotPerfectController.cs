using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class NotPerfectController : Entity
{
	private byte _stage;

	private int _timer;

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (map.BigRingSpawned)
		{
			if (_timer == 120)
			{
				TcpPacket pack = new TcpPacket(PacketType.SERVER_NPCONTROLLER_STATE);
				pack.Write(value: false);
				pack.Write((byte)0);
				pack.Write((byte)0);
				server.TCPMulticast(pack);
			}
			if (_timer >= 300)
			{
				_stage++;
				_timer = 0;
				TcpPacket pack3 = new TcpPacket(PacketType.SERVER_NPCONTROLLER_STATE);
				pack3.Write(value: true);
				pack3.Write((byte)((int)_stage % 4));
				pack3.Write((byte)(Math.Max(_stage - 1, 0) % 4));
				server.TCPMulticast(pack3);
			}
		}
		else
		{
			if (_timer == 900)
			{
				TcpPacket pack4 = new TcpPacket(PacketType.SERVER_NPCONTROLLER_STATE);
				pack4.Write(value: false);
				pack4.Write((byte)0);
				pack4.Write((byte)0);
				server.TCPMulticast(pack4);
			}
			if (_timer >= 1200)
			{
				_stage++;
				_timer = 0;
				TcpPacket pack2 = new TcpPacket(PacketType.SERVER_NPCONTROLLER_STATE);
				pack2.Write(value: true);
				pack2.Write((byte)((int)_stage % 4));
				pack2.Write((byte)(Math.Max(_stage - 1, 0) % 4));
				server.TCPMulticast(pack2);
			}
		}
		_timer++;
		return null;
	}
}
