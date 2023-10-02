using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class MovingSpikeController : Entity
{
	private int _timer = 120;

	private int _frame;

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
		if (_timer-- <= 0)
		{
			_frame++;
			if (_frame > 5)
			{
				_frame = 0;
			}
			if (_frame == 0 || _frame == 2)
			{
				_timer = 120;
			}
			else
			{
				_timer = 0;
			}
			TcpPacket pk = new TcpPacket(PacketType.SERVER_MOVINGSPIKE_STATE, (byte)_frame);
			server.TCPMulticast(pk);
		}
		return null;
	}
}
