using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class LCChainController : Entity
{
	private int _timer;

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
		if (_timer == 480)
		{
			TcpPacket packet = new TcpPacket(PacketType.SERVER_LCCHAIN_STATE);
			packet.Write(0);
			server.TCPMulticast(packet);
		}
		if (_timer == 600)
		{
			TcpPacket packet3 = new TcpPacket(PacketType.SERVER_LCCHAIN_STATE);
			packet3.Write(1);
			server.TCPMulticast(packet3);
		}
		if (_timer >= 720)
		{
			TcpPacket packet2 = new TcpPacket(PacketType.SERVER_LCCHAIN_STATE);
			packet2.Write(2);
			server.TCPMulticast(packet2);
			_timer = 0;
		}
		_timer++;
		return null;
	}
}
