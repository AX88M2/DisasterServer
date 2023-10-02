using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class GHZThunder : Entity
{
	private int _timer;

	private Random _rand = new Random();

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		_timer = 60 * _rand.Next(15, 20);
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_timer == 120)
		{
			TcpPacket pack2 = new TcpPacket(PacketType.SERVER_GHZTHUNDER_STATE, (byte)0);
			server.TCPMulticast(pack2);
		}
		if (_timer <= 0)
		{
			_timer = 60 * _rand.Next(15, 20);
			TcpPacket pack = new TcpPacket(PacketType.SERVER_GHZTHUNDER_STATE, (byte)1);
			server.TCPMulticast(pack);
		}
		_timer--;
		return null;
	}
}
