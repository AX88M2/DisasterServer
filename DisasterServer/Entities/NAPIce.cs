using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class NAPIce : Entity
{
	public byte ID;

	private bool _activated = true;

	private int _timer;

	public NAPIce(byte id)
	{
		ID = id;
	}

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
		if (!_activated)
		{
			return null;
		}
		if (_timer-- <= 0)
		{
			_activated = false;
			_timer = 900;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_NAPICE_STATE, (byte)1, ID));
		}
		return null;
	}

	public void Activate(Server server)
	{
		if (!_activated)
		{
			_timer = 900;
			_activated = true;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_NAPICE_STATE, (byte)0, ID));
		}
	}
}
