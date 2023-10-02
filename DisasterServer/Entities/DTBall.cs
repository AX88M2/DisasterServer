using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class DTBall : Entity
{
	private double _state;

	private bool _side = true;

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
		if (_side)
		{
			_state += 0.014999999664723873;
			if (_state >= 1.0)
			{
				_side = false;
			}
		}
		else
		{
			_state -= 0.014999999664723873;
			if (_state <= -1.0)
			{
				_side = true;
			}
		}
		return new UdpPacket(PacketType.SERVER_DTBALL_STATE, (float)_state);
	}
}
