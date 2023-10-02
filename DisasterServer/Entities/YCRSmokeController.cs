using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class YCRSmokeController : Entity
{
	public int _timer;

	public bool _activated;

	public byte _id;

	private Random _rand = new Random();

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_timer > 360)
		{
			_timer = 0;
			_activated = !_activated;
			if (_activated)
			{
				_id = (byte)_rand.Next(7);
			}
			else
			{
				_id = 0;
			}
			Terminal.LogDebug($"{_id}: state {_activated}");
			TcpPacket packet2 = new TcpPacket(PacketType.SERVER_YCRSMOKE_STATE);
			packet2.Write(_activated);
			packet2.Write(_id);
			server.TCPMulticast(packet2);
		}
		if (_activated && _timer == 60)
		{
			TcpPacket packet = new TcpPacket(PacketType.SERVER_YCRSMOKE_READY);
			packet.Write(_id);
			server.TCPMulticast(packet);
		}
		_timer++;
		return null;
	}
}
