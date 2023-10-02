using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public abstract class Entity
{
	public int X;

	public int Y;

	public abstract TcpPacket? Spawn(Server server, Game game, Map map);

	public abstract UdpPacket? Tick(Server server, Game game, Map map);

	public abstract TcpPacket? Destroy(Server server, Game game, Map map);
}
