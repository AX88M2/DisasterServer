using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class PFLift : Entity
{
	public byte ID;

	public ushort ActivatorID;

	private bool _activated;

	private float _y;

	private float _startY;

	private float _endY;

	private int _timer;

	private float _speed;

	public PFLift(byte id, float starty, float endY)
	{
		ID = id;
		_startY = starty;
		_endY = endY;
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
			if (_timer > 0)
			{
				_timer--;
				if (_timer == 0)
				{
					server.TCPMulticast(new TcpPacket(PacketType.SERVER_PFLIFT_STATE, (byte)3, ID, (ushort)_startY));
				}
			}
			return null;
		}
		if (_y > _endY)
		{
			if (_speed < 7f)
			{
				_speed += 0.052f;
			}
			_y -= _speed;
		}
		else
		{
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_PFLIFT_STATE, (byte)2, ID, ActivatorID, (ushort)_y));
			_timer = 90;
			_activated = false;
			ActivatorID = 0;
		}
		return new UdpPacket(PacketType.SERVER_PFLIFT_STATE, (byte)1, ID, ActivatorID, (ushort)_y);
	}

	public void Activate(Server server, ushort id)
	{
		if (!_activated && _timer <= 0)
		{
			ActivatorID = id;
			_timer = 0;
			_speed = 0f;
			_y = _startY;
			_activated = true;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_PFLIFT_STATE, (byte)0, ID, ActivatorID));
		}
	}
}
