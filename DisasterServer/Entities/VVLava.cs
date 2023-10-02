using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class VVLava : Entity
{
	public float StartY;

	public float TravelDistance;

	public byte ID;

	private byte _state;

	private int _timer = 1200;

	private float _y;

	private float _accel;

	private static Random _rand = new Random();

	public VVLava(byte id, float startY, float dist)
	{
		ID = id;
		StartY = startY;
		_y = startY;
		TravelDistance = dist;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		_timer += _rand.Next(2, 5) * 60;
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		switch (_state)
		{
		case 0:
			_y = StartY + MathF.Sin((float)_timer / 25f) * 6f;
			if (_timer-- <= 0)
			{
				_state = 1;
			}
			break;
		case 1:
			if (_y < StartY + 20f)
			{
				_y += 0.15f;
			}
			else
			{
				_state = 2;
			}
			break;
		case 2:
			if (_y > StartY - TravelDistance)
			{
				if (_accel < 5f)
				{
					_accel += 0.08f;
				}
				_y -= _accel;
			}
			else
			{
				_state = 3;
				_timer = 300;
				_accel = 0f;
			}
			break;
		case 3:
			if (_timer-- <= 0)
			{
				_state = 4;
			}
			_y = StartY - TravelDistance + MathF.Sin((float)_timer / 25f) * 6f;
			break;
		case 4:
			if (StartY > _y)
			{
				if (_accel < 5f)
				{
					_accel += 0.08f;
				}
				_y += _accel;
			}
			else
			{
				_state = 0;
				_timer = 1200;
				_accel = 0f;
			}
			break;
		}
		return new UdpPacket(PacketType.SERVER_VVLCOLUMN_STATE, ID, _state, _y);
	}
}
