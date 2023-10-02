using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class RMZSlime : Entity
{
	public byte State;

	public byte ID;

	public int SpawnX;

	public int SpawnY;

	private float _x;

	private bool _state;

	private byte _ring;

	private Random _rand = new Random();

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		int rng = _rand.Next(100);
		if (rng >= 0 && rng < 50)
		{
			_ring = 0;
		}
		else if (rng >= 40 && rng < 90)
		{
			_ring = 1;
		}
		else
		{
			_ring = 2;
		}
		switch (_ring)
		{
		case 0:
			State = 0;
			break;
		case 1:
			State = 2;
			break;
		case 2:
			State = 4;
			break;
		}
		return new TcpPacket(PacketType.SERVER_RMZSLIME_STATE, (byte)0, ID, (ushort)X, (ushort)Y, State);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_RMZSLIME_STATE, (byte)2, ID);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		UpdateState();
		return new UdpPacket(PacketType.SERVER_RMZSLIME_STATE, (byte)1, ID, (ushort)X, (ushort)Y, State);
	}

	private void UpdateState()
	{
		if (_state)
		{
			_x -= 1f;
		}
		else
		{
			_x += 1f;
		}
		X = (ushort)((float)SpawnX + _x);
		if (_x > 100f)
		{
			switch (_ring)
			{
			case 0:
				State = 1;
				break;
			case 1:
				State = 3;
				break;
			case 2:
				State = 5;
				break;
			}
			_state = true;
		}
		else if (_x < -100f)
		{
			switch (_ring)
			{
			case 0:
				State = 0;
				break;
			case 1:
				State = 2;
				break;
			case 2:
				State = 4;
				break;
			}
			_state = false;
		}
	}
}
