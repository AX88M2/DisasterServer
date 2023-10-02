using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class RMZSlimeSpawner : Entity
{
	public int ID = -1;

	public bool HasSlime;

	public RMZSlime Slime;

	public const int SPAWN_INTERVAL = 900;

	private int _timer = 900;

	private Random _rand = new Random();

	private static byte _slimeIds;

	private object _lock = new object();

	public RMZSlimeSpawner(int x, int y)
	{
		X = x;
		Y = y;
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		lock (_lock)
		{
			_timer += _rand.Next(2, 17) * 60;
		}
		return null;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		lock (_lock)
		{
			if (HasSlime)
			{
				return null;
			}
			if (_timer-- > 0)
			{
				return null;
			}
			Slime = map.Spawn(server, new RMZSlime
			{
				X = X,
				Y = Y,
				SpawnX = X,
				SpawnY = Y,
				ID = _slimeIds++
			});
			HasSlime = true;
			ID = Slime.ID;
		}
		return null;
	}

	public void KillSlime(Server server, Map map, Peer killer, bool isProjectile)
	{
		lock (_lock)
		{
			if (!HasSlime)
			{
				return;
			}
			switch (Slime.State)
			{
			case 2:
			case 3:
				if (!isProjectile)
				{
					SharedServerSession sess = server.GetSession(killer.ID);
					server.TCPSend(sess, new TcpPacket(PacketType.SERVER_RMZSLIME_RINGBONUS, false));
				}
				break;
			case 4:
			case 5:
				if (!isProjectile)
				{
					SharedServerSession sess2 = server.GetSession(killer.ID);
					server.TCPSend(sess2, new TcpPacket(PacketType.SERVER_RMZSLIME_RINGBONUS, true));
				}
				break;
			}
			map.Destroy(server, Slime);
			HasSlime = false;
			_timer = 900 + _rand.Next(2, 17) * 60;
			ID = -1;
		}
	}
}
