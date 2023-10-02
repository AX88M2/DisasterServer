using System.Net;
using DisasterServer.Data;
using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class RavineMist : Map
{
	private readonly Vector2[] _slimeSpawnPoints = new Vector2[11]
	{
		new Vector2(1901, 392),
		new Vector2(2193, 392),
		new Vector2(2468, 392),
		new Vector2(1188, 860),
		new Vector2(2577, 1952),
		new Vector2(2564, 2264),
		new Vector2(2782, 2264),
		new Vector2(1441, 2264),
		new Vector2(884, 2264),
		new Vector2(988, 2004),
		new Vector2(915, 2004)
	};

	private readonly Vector2[] _shardSpawnPoints = new Vector2[12]
	{
		new Vector2(862, 248),
		new Vector2(3078, 248),
		new Vector2(292, 558),
		new Vector2(2918, 558),
		new Vector2(1100, 820),
		new Vector2(980, 1188),
		new Vector2(1870, 1252),
		new Vector2(2180, 1508),
		new Vector2(2920, 2216),
		new Vector2(282, 2228),
		new Vector2(1318, 1916),
		new Vector2(3010, 1766)
	};

	private Dictionary<ushort, byte> _playersShardCount = new Dictionary<ushort, byte>();

	private Random _rand = new Random();

	private int _ringLoc;

	public override void Init(Server server)
	{
		IEnumerable<Vector2> points = _shardSpawnPoints.OrderBy((Vector2 e) => _rand.Next()).Take(7);
		lock (Entities)
		{
			foreach (Vector2 point in points)
			{
				Spawn(server, new RMZShard(point.X, point.Y));
			}
			Vector2[] slimeSpawnPoints = _slimeSpawnPoints;
			foreach (Vector2 coord in slimeSpawnPoints)
			{
				Spawn(server, new RMZSlimeSpawner(coord.X, coord.Y));
			}
		}
		lock (server.Peers)
		{
			foreach (KeyValuePair<ushort, Peer> peer in server.Peers)
			{
				if (!peer.Value.Waiting)
				{
					_playersShardCount.Add(peer.Key, 0);
				}
			}
		}
		_ringLoc = _rand.Next(255);
		SetTime(server, 180);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	public override void PeerLeft(Server server, TcpSession session, Peer peer)
	{
		lock (_playersShardCount)
		{
			lock (server.Peers)
			{
				lock (Entities)
				{
					for (int i = 0; i < _playersShardCount[session.ID]; i++)
					{
						Spawn(server, new RMZShard((int)(peer.Player.X + (float)_rand.Next(-8, 8)), (int)peer.Player.Y, spawned: true));
					}
				}
				_playersShardCount.Remove(session.ID);
			}
		}
		SendRingState(server);
		base.PeerLeft(server, session, peer);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		switch ((PacketType)reader.ReadByte())
		{
		case PacketType.CLIENT_PLAYER_DEATH_STATE:
			lock (Entities)
			{
				bool num = !reader.ReadBoolean();
				reader.ReadByte();
				if (num)
				{
					break;
				}
				lock (_playersShardCount)
				{
					lock (server.Peers)
					{
						for (int i = 0; i < _playersShardCount[session.ID]; i++)
						{
							Spawn(server, new RMZShard((int)(server.Peers[session.ID].Player.X + (float)_rand.Next(-8, 8)), (int)server.Peers[session.ID].Player.Y, spawned: true));
						}
						_playersShardCount[session.ID] = 0;
					}
				}
				SendRingState(server);
			}
			break;
		case PacketType.CLIENT_RMZSLIME_HIT:
			lock (Entities)
			{
				byte id = reader.ReadByte();
				ushort pid = reader.ReadUInt16();
				bool isProj = reader.ReadBoolean();
				Terminal.Log($"Killing slime {id}");
				RMZSlimeSpawner[] slimes = FindOfType<RMZSlimeSpawner>();
				if (slimes == null)
				{
					break;
				}
				RMZSlimeSpawner slime = slimes.Where((RMZSlimeSpawner e) => e.ID == id).FirstOrDefault();
				if (slime == null)
				{
					break;
				}
				Peer killer;
				lock (server.Peers)
				{
					killer = server.Peers.Values.Where((Peer e) => e.ID == pid && !e.Waiting).FirstOrDefault();
				}
				if (killer != null)
				{
					slime.KillSlime(server, this, killer, isProj);
					Terminal.Log($"Killed slime {id}");
				}
			}
			break;
		case PacketType.CLIENT_RMZSHARD_COLLECT:
			lock (Entities)
			{
				byte gid = reader.ReadByte();
				RMZShard[] list = FindOfType<RMZShard>();
				if (list == null)
				{
					return;
				}
				RMZShard ent = list.FirstOrDefault((RMZShard e) => e.ID == gid);
				if (ent == null)
				{
					return;
				}
				lock (_shardSpawnPoints)
				{
					_playersShardCount[session.ID]++;
				}
				server.TCPMulticast(new TcpPacket(PacketType.SERVER_RMZSHARD_STATE, (byte)2, ent.ID, session.ID));
				Destroy(server, ent);
				SendRingState(server);
			}
			break;
		}
		base.PeerTCPMessage(server, session, reader);
	}

	public override void PeerUDPMessage(Server server, IPEndPoint endpoint, byte[] data)
	{
		base.PeerUDPMessage(server, endpoint, data);
	}

	protected override void DoBigRingTimer(Server server)
	{
		if (Timer - 3600 <= 0 && !BigRingSpawned)
		{
			TcpPacket packet = new TcpPacket(PacketType.SERVER_GAME_SPAWN_RING);
			packet.Write(value: false);
			packet.Write((byte)_ringLoc);
			server.TCPMulticast(packet);
			BigRingSpawned = true;
		}
		if (Timer - RingActivateTime > 0 || BigRingReady)
		{
			return;
		}
		lock (Entities)
		{
			int count = 7 - Entities.Count((Entity e) => e is RMZShard);
			TcpPacket packet2 = new TcpPacket(PacketType.SERVER_GAME_SPAWN_RING);
			packet2.Write(count >= 6);
			packet2.Write((byte)_ringLoc);
			server.TCPMulticast(packet2);
			BigRingSpawned = true;
		}
	}

	public void SendRingState(Server server)
	{
		lock (Entities)
		{
			int count = 7 - Entities.Count((Entity e) => e is RMZShard);
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_RMZSHARD_STATE, (byte)3, (byte)count));
			if (Timer - RingActivateTime <= 0 && !BigRingReady)
			{
				TcpPacket packet = new TcpPacket(PacketType.SERVER_GAME_SPAWN_RING);
				packet.Write(count >= 6);
				packet.Write((byte)_ringLoc);
				server.TCPMulticast(packet);
				BigRingSpawned = true;
			}
		}
	}

	protected override int GetRingSpawnCount()
	{
		return 27;
	}
}
