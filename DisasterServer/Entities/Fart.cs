using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class Fart : Entity
{
	private float _xspd;

	private float _x;

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		X = 1616;
		Y = 2608;
		_x = X;
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		_x += _xspd;
		_x = Math.Clamp(_x, 1282f, 2944f);
		_xspd -= MathF.Min(MathF.Abs(_xspd), 0.1875f) * (float)MathF.Sign(_xspd);
		return new UdpPacket(PacketType.SERVER_FART_STATE, (ushort)_x, (ushort)Y);
	}

	public void Push(sbyte force)
	{
		_xspd = force;
	}
}
