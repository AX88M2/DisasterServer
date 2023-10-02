using System.Net;
using DisasterServer.Data;
using DisasterServer.Entities;
using DisasterServer.Session;
using DisasterServer.State;
using ExeNet;

namespace DisasterServer.Maps;

public abstract class Map
{
	public Game Game;

	public int Timer = 7200;

	public List<Entity> Entities = new List<Entity>();

	public bool BigRingSpawned;

	public bool BigRingReady;

	public ushort RingIDs = 1;

	public ushort BRingsIDs = 1;

	public byte ExellerCloneIDs = 1;

	protected int RingActivateTime = 3000;

	private float _ringCoff;

	private int _ringTimer = -240;

	private Random _rand = new Random();

	private bool[] _ringSpawns;

	public virtual void Init(Server server)
	{
		_ringCoff = GetRingTime();
		lock (server.Peers)
		{
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) > 3)
			{
				_ringCoff -= 1f;
			}
		}
		_ringSpawns = new bool[GetRingSpawnCount()];
		TcpPacket pack = new TcpPacket(PacketType.SERVER_GAME_PLAYERS_READY);
		server.TCPMulticast(pack);
	}

	public virtual void Tick(Server server)
	{
		if (Timer % 60 == 0)
		{
			TcpPacket packet2 = new TcpPacket(PacketType.SERVER_GAME_TIME_SYNC);
			packet2.Write((ushort)Timer);
			server.TCPMulticast(packet2);
		}
		DoRingTimer(server);
		DoBigRingTimer(server);
		if (Timer > 0)
		{
			Timer--;
		}
		lock (Entities)
		{
			for (int i = 0; i < Entities.Count; i++)
			{
				UdpPacket packet = Entities[i].Tick(server, Game, this);
				if (packet != null)
				{
					server.UDPMulticast(ref Game.IPEndPoints, packet);
				}
			}
		}
	}

	public virtual void PeerLeft(Server server, TcpSession session, Peer peer)
	{
	}

	public virtual void PeerUDPMessage(Server server, IPEndPoint endpoint, byte[] data)
	{
	}

	public virtual void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.BaseStream.Seek(0L, SeekOrigin.Begin);
		reader.ReadBoolean();
		switch ((PacketType)reader.ReadByte())
		{
		case PacketType.CLIENT_TPROJECTILE:
			if (FindOfType<TailsProjectile>().Length == 0)
			{
				TailsProjectile projectile = new TailsProjectile
				{
					OwnerID = session.ID,
					X = reader.ReadUInt16(),
					Y = reader.ReadUInt16(),
					Direction = reader.ReadSByte(),
					Damage = reader.ReadByte(),
					IsExe = reader.ReadBoolean(),
					Charge = reader.ReadByte()
				};
				Spawn(server, projectile);
			}
			break;
		case PacketType.CLIENT_TPROJECTILE_HIT:
			Destroy<TailsProjectile>(server);
			break;
		case PacketType.CLIENT_ETRACKER:
		{
			EggmanTracker tracker = new EggmanTracker
			{
				X = reader.ReadUInt16(),
				Y = reader.ReadUInt16()
			};
			Spawn(server, tracker);
			break;
		}
		case PacketType.CLIENT_ETRACKER_ACTIVATED:
			lock (Entities)
			{
				byte id = reader.ReadByte();
				ushort activator = reader.ReadUInt16();
				EggmanTracker[] trackers = FindOfType<EggmanTracker>();
				if (trackers != null)
				{
					EggmanTracker tracker2 = trackers.Where((EggmanTracker e) => e.ID == id).FirstOrDefault();
					if (tracker2 != null)
					{
						tracker2.ActivatorID = activator;
						Destroy(server, tracker2);
					}
				}
				break;
			}
		case PacketType.CLIENT_CREAM_SPAWN_RINGS:
		{
			ushort _x = reader.ReadUInt16();
			ushort _y = reader.ReadUInt16();
			if (reader.ReadBoolean())
			{
				for (int j = 0; j < 2; j++)
				{
					Spawn(server, new CreamRing
					{
						X = (int)((double)(int)_x + Math.Sin(7.853981633974483 - (double)j * Math.PI) * 26.0) - 1,
						Y = (int)((double)(int)_y + Math.Cos(7.853981633974483 - (double)j * Math.PI) * 26.0),
						IsRedRing = true
					});
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					Spawn(server, new CreamRing
					{
						X = (int)((double)(int)_x + Math.Sin(7.853981633974483 + (double)i * (Math.PI / 2.0)) * 26.0),
						Y = (int)((double)(int)_y + Math.Cos(7.853981633974483 + (double)i * (Math.PI / 2.0)) * 26.0),
						IsRedRing = false
					});
				}
			}
			break;
		}
		case PacketType.CLIENT_RING_COLLECTED:
			lock (Entities)
			{
				reader.ReadByte();
				ushort uid = reader.ReadUInt16();
				Ring[] rings = FindOfType<Ring>();
				if (rings != null)
				{
					Ring ring = rings.Where((Ring e) => e.ID == uid).FirstOrDefault();
					if (ring != null)
					{
						TcpPacket packet = new TcpPacket(PacketType.SERVER_RING_COLLECTED, ring.IsRedRing);
						server.TCPSend(session, packet);
						Destroy(server, ring);
					}
				}
				break;
			}
		case PacketType.CLIENT_BRING_COLLECTED:
			lock (Entities)
			{
				ushort uid2 = reader.ReadUInt16();
				BlackRing[] rings2 = FindOfType<BlackRing>();
				if (rings2 != null)
				{
					BlackRing ring2 = rings2.Where((BlackRing e) => e.ID == uid2).FirstOrDefault();
					if (ring2 != null)
					{
						TcpPacket packet2 = new TcpPacket(PacketType.SERVER_BRING_COLLECTED);
						server.TCPSend(session, packet2);
						Destroy(server, ring2);
					}
				}
				break;
			}
		case PacketType.CLIENT_ERECTOR_BRING_SPAWN:
		{
			float x = reader.ReadSingle();
			float y = reader.ReadSingle();
			BlackRing inst = Spawn<BlackRing>(server, callSpawn: false);
			inst.ID = BRingsIDs++;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_ERECTOR_BRING_SPAWN, inst.ID, x, y));
			break;
		}
		case PacketType.CLIENT_EXELLER_SPAWN_CLONE:
			if (FindOfType<ExellerClone>().Length <= 1)
			{
				ushort x2 = reader.ReadUInt16();
				ushort y2 = reader.ReadUInt16();
				sbyte dir = reader.ReadSByte();
				Spawn(server, new ExellerClone(session.ID, ExellerCloneIDs++, x2, y2, dir));
			}
			break;
		case PacketType.CLIENT_EXELLER_TELEPORT_CLONE:
		{
			byte uid3 = reader.ReadByte();
			ExellerClone[] clones = FindOfType<ExellerClone>();
			if (clones != null)
			{
				ExellerClone clone = clones.Where((ExellerClone e) => e.ID == uid3).FirstOrDefault();
				if (clone != null)
				{
					Destroy(server, clone);
				}
			}
			break;
		}
		}
	}

	public void SetTime(Server server, int seconds)
	{
		Timer = seconds * 60 + GetPlayerOffset(server) * 60;
		Terminal.LogDebug($"Timer is set to {Timer} frames");
	}

	public void ActivateRingAfter(int afterSeconds)
	{
		RingActivateTime = 3600 - afterSeconds * 60;
		Terminal.LogDebug($"Ring activate time is set to {Timer} frames");
	}

	public void FreeRingID(byte iid)
	{
		lock (_ringSpawns)
		{
			_ringSpawns[iid] = false;
		}
	}

	public bool GetFreeRingID(out byte iid)
	{
		lock (_ringSpawns)
		{
			if (_ringSpawns.Where((bool e) => !e).Count() <= 0)
			{
				iid = 0;
				return false;
			}
			byte rn;
			do
			{
				rn = (byte)_rand.Next(_ringSpawns.Length);
			}
			while (_ringSpawns[rn]);
			_ringSpawns[rn] = true;
			iid = rn;
			return true;
		}
	}

	public T? Spawn<T>(Server server, bool callSpawn = true) where T : Entity
	{
		T entity = Ext.CreateOfType<T>();
		if (entity == null)
		{
			return null;
		}
		TcpPacket pack = null;
		lock (Entities)
		{
			Entities.Add(entity);
			if (callSpawn)
			{
				pack = entity.Spawn(server, Game, this);
			}
		}
		if (pack != null)
		{
			server.TCPMulticast(pack);
		}
		Terminal.LogDebug($"Entity {entity} spawned.");
		return entity;
	}

	public T Spawn<T>(Server server, T entity, bool callSpawn = true) where T : Entity
	{
		TcpPacket pack = null;
		lock (Entities)
		{
			Entities.Add(entity);
			if (callSpawn)
			{
				pack = entity.Spawn(server, Game, this);
			}
		}
		if (pack != null)
		{
			server.TCPMulticast(pack);
		}
		Terminal.LogDebug($"Entity {entity} spawned.");
		return entity;
	}

	public void Destroy(Server server, Entity entity)
	{
		TcpPacket pack;
		lock (Entities)
		{
			Entities.Remove(entity);
			pack = entity.Destroy(server, Game, this);
		}
		if (pack != null)
		{
			server.TCPMulticast(pack);
		}
		Terminal.LogDebug($"Entity {entity} destroyed.");
	}

	public void Destroy<T>(Server server) where T : Entity
	{
		lock (Entities)
		{
			T[] pick = FindOfType<T>();
			if (pick != null)
			{
				T[] array = pick;
				foreach (T p in array)
				{
					Destroy(server, p);
					Terminal.LogDebug($"Entity {p} destroyed.");
				}
			}
		}
	}

	public T[]? FindOfType<T>() where T : Entity
	{
		lock (Entities)
		{
			Entity[] pick = Entities.Where((Entity e) => e is T).ToArray();
			Terminal.LogDebug($"Entity search found {pick.Length} entities of type {typeof(T).FullName}");
			return Array.ConvertAll(pick, (Entity e) => (T)e);
		}
	}

	private void DoRingTimer(Server server)
	{
		if ((float)_ringTimer >= _ringCoff * 60f)
		{
			_ringTimer = 0;
			if (!GetFreeRingID(out var iid))
			{
				return;
			}
			Spawn(server, new Ring
			{
				IID = iid
			});
		}
		_ringTimer++;
	}

	protected virtual void DoBigRingTimer(Server server)
	{
		if (Timer - 3600 <= 0 && !BigRingSpawned)
		{
			TcpPacket packet = new TcpPacket(PacketType.SERVER_GAME_SPAWN_RING);
			packet.Write(value: false);
			packet.Write((byte)_rand.Next(255));
			server.TCPMulticast(packet);
			BigRingSpawned = true;
		}
		int min = RingActivateTime;
		if (Timer - min <= 0 && !BigRingReady)
		{
			TcpPacket packet2 = new TcpPacket(PacketType.SERVER_GAME_SPAWN_RING);
			packet2.Write(value: true);
			packet2.Write((byte)_rand.Next(255));
			server.TCPMulticast(packet2);
			BigRingSpawned = true;
		}
	}

	public virtual bool CanSpawnRedRings()
	{
		return true;
	}

	protected virtual int GetPlayerOffset(Server server)
	{
		lock (server.Peers)
		{
			return (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) - 1) * 20;
		}
	}

	protected virtual float GetRingTime()
	{
		return 5f;
	}

	protected abstract int GetRingSpawnCount();
}
