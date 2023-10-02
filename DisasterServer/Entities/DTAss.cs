using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class DTAss : Entity
{
	public static byte SID;

	public byte ID;

	private bool _state;

	private double _accel;

	private double _y;

	private int _sY;

	private int _timer = -60;

	public DTAss(int x, int y)
	{
		X = x;
		Y = y;
		_y = y;
		_sY = y;
		ID = SID++;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_DTASS_STATE, (byte)0, ID, (ushort)X, (ushort)Y);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_timer == 0)
		{
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_DTASS_STATE, (byte)0, ID, (ushort)X, (ushort)Y));
		}
		if (_timer > -60)
		{
			_timer--;
		}
		if (_timer <= -60 && !_state)
		{
			lock (server.Peers)
			{
				foreach (KeyValuePair<ushort, Peer> peer in server.Peers)
				{
					if (!peer.Value.Waiting && !peer.Value.Player.Invisible)
					{
						float dist = peer.Value.Player.Y - (float)Y;
						if (dist > 0f && dist <= 336f && peer.Value.Player.X >= (float)X && peer.Value.Player.X <= (float)(X + 80))
						{
							server.TCPMulticast(new TcpPacket(PacketType.SERVER_DTASS_STATE, (byte)2, ID));
							_state = true;
							break;
						}
					}
				}
			}
		}
		if (_state)
		{
			_accel += 0.164;
			_y += _accel;
			Y = (int)_y;
			return new UdpPacket(PacketType.SERVER_DTASS_STATE, ID, (ushort)X, (ushort)Y);
		}
		return null;
	}

	public void Dectivate(Server server)
	{
		_state = false;
		_y = _sY;
		Y = _sY;
		_timer = 1500;
		_accel = 0.0;
		server.TCPMulticast(new TcpPacket(PacketType.SERVER_DTASS_STATE, (byte)1, ID));
	}
}
