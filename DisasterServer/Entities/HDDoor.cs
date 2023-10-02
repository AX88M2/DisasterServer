using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class HDDoor : Entity
{
	public bool _state;

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
		if (_timer > 0)
		{
			_timer--;
			if (_timer == 0)
			{
				server.TCPMulticast(new TcpPacket(PacketType.SERVER_HDDOOR_STATE, (byte)1, true));
			}
		}
		return null;
	}

	public void Toggle(Server server)
	{
		if (_timer <= 0)
		{
			_state = !_state;
			_timer = 600;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_HDDOOR_STATE, (byte)0, _state));
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_HDDOOR_STATE, (byte)1, false));
		}
	}
}
