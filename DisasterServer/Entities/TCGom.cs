using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class TCGom : Entity
{
	private int _timer = 240;

	private int _id;

	private bool _state;

	private Random _rand = new Random();

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
		if (_timer-- <= 0)
		{
			_timer = 240;
			_state = !_state;
			if (_state)
			{
				_id = _rand.Next(7);
			}
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_TCGOM_STATE, (byte)_id, _state));
		}
		return null;
	}
}
