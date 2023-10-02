using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class DTTailsDoll : Entity
{
	private int _target = -1;

	private int _timer;

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		FindSpot(server);
		return new TcpPacket(PacketType.SERVER_DTTAILSDOLL_STATE, (byte)0, (ushort)X, (ushort)Y, _target == -1 && _timer <= 0);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_timer > 0)
		{
			if (_timer == 1)
			{
				server.TCPSend(server.GetSession((ushort)_target), new TcpPacket(PacketType.SERVER_DTTAILSDOLL_STATE, (byte)3));
			}
			_timer--;
		}
		lock (server.Peers)
		{
			if (_timer <= 0 && (_target == -1 || !server.Peers.ContainsKey((ushort)_target)))
			{
				foreach (Peer player2 in server.Peers.Values)
				{
					if (player2.Player.Character != 0 && player2.Player.RevivalTimes < 2 && !player2.Waiting && !player2.Player.HasEscaped && Ext.Dist(player2.Player.X, player2.Player.Y, X, Y) < 130.0)
					{
						_target = player2.ID;
						_timer = 60;
						server.TCPSend(server.GetSession(player2.ID), new TcpPacket(PacketType.SERVER_DTTAILSDOLL_STATE, (byte)2));
						break;
					}
				}
			}
			if (_target != -1 && _timer <= 0)
			{
				foreach (Peer player in server.Peers.Values)
				{
					if (player.Player.Character != 0 && player.Player.RevivalTimes < 2 && !player.Waiting && Ext.Dist(player.Player.X, player.Player.Y, X, Y) < 80.0)
					{
						_target = player.ID;
						break;
					}
				}
				if (!server.Peers.ContainsKey((ushort)_target))
				{
					FindSpot(server);
					_target = -1;
				}
				else
				{
					Player plr = server.Peers[(ushort)_target].Player;
					if (plr.HasEscaped)
					{
						FindSpot(server);
						_target = -1;
					}
					else
					{
						if ((int)Math.Abs(plr.X - (float)X) >= 4)
						{
							X += Math.Sign((int)plr.X - X) * 4;
						}
						if ((int)Math.Abs(plr.Y - (float)Y) >= 2)
						{
							Y += Math.Sign((int)plr.Y - Y) * 2;
						}
						if (Ext.Dist(plr.X, plr.Y, X, Y) < 18.0)
						{
							server.TCPSend(server.GetSession((ushort)_target), new TcpPacket(PacketType.SERVER_DTTAILSDOLL_STATE, (byte)1));
							FindSpot(server);
							_target = -1;
						}
					}
				}
			}
		}
		byte state = 0;
		if (_target == -1)
		{
			state = 0;
		}
		if (_target != -1 && _timer > 0)
		{
			state = 1;
		}
		if (_target != -1 && _timer <= 0)
		{
			state = 2;
		}
		return new UdpPacket(PacketType.SERVER_DTTAILSDOLL_STATE, (ushort)X, (ushort)Y, state);
	}

	private void FindSpot(Server server)
	{
		Vector2[] pos = new Vector2[11]
		{
			new Vector2(177, 944),
			new Vector2(1953, 544),
			new Vector2(3279, 224),
			new Vector2(4101, 544),
			new Vector2(4060, 1264),
			new Vector2(3805, 1824),
			new Vector2(2562, 1584),
			new Vector2(515, 1824),
			new Vector2(2115, 1056),
			new Vector2(984, 1184),
			new Vector2(1498, 1504)
		};
		List<Vector2> choosen = new List<Vector2>();
		Vector2[] array = pos;
		foreach (Vector2 p in array)
		{
			choosen.Add(p);
		}
		lock (server.Peers)
		{
			array = pos;
			foreach (Vector2 p2 in array)
			{
				foreach (Peer player in server.Peers.Values)
				{
					if (Ext.Dist(player.Player.X, player.Player.Y, p2.X, p2.Y) < 480.0)
					{
						choosen.Remove(p2);
						break;
					}
				}
			}
			if (choosen.Count > 0)
			{
				Vector2 point2 = choosen[new Random().Next(choosen.Count)];
				X = point2.X;
				Y = point2.Y;
				Terminal.LogDebug($"Tails doll found spot at ({point2.X}, {point2.Y})");
			}
			else
			{
				Vector2 point = pos[new Random().Next(choosen.Count)];
				X = point.X;
				Y = point.Y;
				Terminal.LogDebug($"Tails doll didn't find a spot, using ({point.X}, {point.Y})");
			}
		}
	}
}
