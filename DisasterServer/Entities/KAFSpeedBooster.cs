using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class KAFSpeedBooster : Entity
{
	public byte ID;

	private int _timer;

	private bool _activated;

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_KAFMONITOR_STATE, (byte)0, ID);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_timer > 0)
		{
			_timer--;
		}
		else if (_timer == 0)
		{
			_activated = false;
			TcpPacket pack = new TcpPacket(PacketType.SERVER_KAFMONITOR_STATE);
			pack.Write((byte)1);
			pack.Write(ID);
			server.TCPMulticast(pack);
			_timer = -1;
		}
		return null;
	}

	public void Activate(Server server, ushort nid, bool isProjectile)
	{
		if (!_activated)
		{
			TcpPacket pack = new TcpPacket(PacketType.SERVER_KAFMONITOR_STATE);
			pack.Write((byte)2);
			pack.Write(ID);
			pack.Write((!isProjectile) ? nid : 0);
			server.TCPMulticast(pack);
			_activated = true;
			_timer = 1800;
		}
	}
}
