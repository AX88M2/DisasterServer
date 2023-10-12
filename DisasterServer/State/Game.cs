using System.Net;
using System.Net.Sockets;
using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.State;

public class Game : State
{
	public Dictionary<ushort, IPEndPoint> IPEndPoints = new Dictionary<ushort, IPEndPoint>();

	private bool _waiting = true;

	private Map _map;

	private readonly ushort _exeId;

	private int _endTimer = -1;

	private int _demonCount;

	private int _timeout = 60;

	private bool _initMap;

	private Dictionary<ushort, int> _lastPackets = new Dictionary<ushort, int>();

	private Dictionary<ushort, int> _packetTimeouts = new Dictionary<ushort, int>();

	private Dictionary<ushort, RevivalData> _reviveTimer = new Dictionary<ushort, RevivalData>();

	public Game(Map map, ushort exe)
	{
		_map = map;
		_map.Game = this;
		_exeId = exe;
	}

	public override DisasterServer.Session.State AsState()
	{
		return DisasterServer.Session.State.GAME;
	}

	public override void PeerJoined(Server server, TcpSession session, Peer peer)
	{
	}

	public override void PeerLeft(Server server, TcpSession session, Peer peer)
	{
		lock (IPEndPoints)
		{
			IPEndPoints.Remove(peer.ID);
		}
		lock (_lastPackets)
		{
			_lastPackets.Remove(peer.ID);
		}
		lock (server.Peers)
		{
			if (peer.Player.RevivalTimes >= 2)
			{
				_demonCount--;
			}
			CheckEscapedAndAlive(server);
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) <= 1)
			{
				server.SetState<Lobby>();
			}
		}
		_map.PeerLeft(server, session, peer);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		Terminal.LogDebug("HandlePlayers()");
		HandlePlayers(server, session, reader);
		Terminal.LogDebug("HandleMap()");
		reader.BaseStream.Seek(0L, SeekOrigin.Begin);
		_map.PeerTCPMessage(server, session, reader);
	}

	public override void PeerUDPMessage(Server server, IPEndPoint endpoint, ref byte[] data)
	{
		IPEndPoint endpoint2 = endpoint;
		try
		{
			if (data.Length == 0)
			{
				Terminal.LogDebug($"Length is 0 from {endpoint2}");
				return;
			}
			FastBitReader reader = new FastBitReader();
			ushort pid = reader.ReadUShort(ref data);
			PacketType type = (PacketType)reader.ReadByte(ref data);
			reader.Position = 3;
			_lastPackets[pid] = 0;
			switch (type)
			{
			case PacketType.CLIENT_PING:
			{
				if (!IPEndPoints.Any<KeyValuePair<ushort, IPEndPoint>>((KeyValuePair<ushort, IPEndPoint> e) => e.Value.ToString() == endpoint2.ToString()) && !_waiting)
				{
					SharedServerSession session = server.GetSession(pid);
					if (session != null)
					{
						Terminal.LogDebug("ANTI SERG BOM BOM");
						server.DisconnectWithReason(session, "invalid session");
					}
					break;
				}
				Terminal.LogDebug($"Ping-pong with {endpoint2} (PID {pid})");
				UdpPacket pk = new UdpPacket(PacketType.SERVER_PONG);
				ulong ping = reader.ReadULong(ref data);
				ushort calc = reader.ReadUShort(ref data);
				pk.Write(ping);
				server.UDPSend(endpoint2, pk);
				pk = new UdpPacket(PacketType.SERVER_GAME_PING);
				pk.Write(pid);
				pk.Write(calc);
				server.UDPMulticast(ref IPEndPoints, pk, endpoint2);
				break;
			}
			case PacketType.CLIENT_PLAYER_DATA:
			{
				if (_waiting)
				{
					break;
				}
				ushort id = reader.ReadUShort(ref data);
				float x = reader.ReadFloat(ref data);
				float y = reader.ReadFloat(ref data);
				lock (server.Peers)
				{
					if (server.Peers.TryGetValue(id, out Peer value) && value != null)
					{
						if (value.Player.Character == Character.Exe && value.Player.ExeCharacter == ExeCharacter.Original)
						{
							reader.ReadByte(ref data);
							reader.ReadUShort(ref data);
							reader.ReadByte(ref data);
							reader.ReadChar(ref data);
							reader.ReadByte(ref data);
							reader.ReadByte(ref data);
							reader.ReadByte(ref data);
							value.Player.Invisible = reader.ReadBoolean(ref data);
						}
						value.Player.X = x;
						value.Player.Y = y;
					}
				}
				reader.Position = 3;
				UdpPacket pack = new UdpPacket(type);
				while (reader.Position < data.Length)
				{
					pack.Write(reader.ReadByte(ref data));
				}
				server.UDPMulticast(ref IPEndPoints, pack, endpoint2);
				break;
			}
			}
			if (_waiting)
			{
				lock (IPEndPoints)
				{
					if (!IPEndPoints.ContainsKey(pid))
					{
						Terminal.LogDebug($"Received from {endpoint2} (PID {pid})");
						IPEndPoints.Add(pid, endpoint2);
						lock (_packetTimeouts)
						{
							_packetTimeouts[pid] = -1;
						}
					}
					lock (server.Peers)
					{
						if (IPEndPoints.Count >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting))
						{
							lock (_map)
							{
								if (!_initMap)
								{
									_map.Init(server);
									_initMap = true;
								}
							}
							Terminal.LogDiscord("Got packets from all players.");
							_waiting = false;
						}
						return;
					}
				}
			}
			reader.Position = 0;
			_map.PeerUDPMessage(server, endpoint2, data);
		}
		catch (Exception e2)
		{
			Terminal.LogDebug($"PeerUDPMessage() failed for {endpoint2}: {e2}");
		}
	}

	public override void UDPSocketError(IPEndPoint endpoint, SocketError error)
	{
		IPEndPoint endpoint2 = endpoint;
		Terminal.LogDebug($"Removing {endpoint2}: {error}");
		lock (IPEndPoints)
		{
			KeyValuePair<ushort, IPEndPoint> item = IPEndPoints.FirstOrDefault<KeyValuePair<ushort, IPEndPoint>>((KeyValuePair<ushort, IPEndPoint> kvp) => kvp.Value == endpoint2);
			IPEndPoints.Remove(item.Key);
		}
		base.UDPSocketError(endpoint2, error);
	}

	public override void Init(Server server)
	{
		lock (server.Peers)
		{
			foreach (Peer peer in server.Peers.Values)
			{
				if (!peer.Waiting)
				{
					lock (_lastPackets)
					{
						_lastPackets.Add(peer.ID, 0);
					}
					lock (_packetTimeouts)
					{
						_packetTimeouts.Add(peer.ID, 1080);
					}
					lock (_reviveTimer)
					{
						_reviveTimer.Add(peer.ID, new RevivalData());
					}
				}
			}
		}
		TcpPacket packet = new TcpPacket(PacketType.SERVER_LOBBY_GAME_START);
		server.TCPMulticast(packet);
		Terminal.LogDiscord("Waiting for players...");
		Program.Stat?.MulticastInformation();
	}

	public override void Tick(Server server)
	{
		if (_endTimer > 0)
		{
			_endTimer--;
			return;
		}
		if (_endTimer == 0)
		{
			server.SetState<Lobby>();
			return;
		}
		if (_waiting)
		{
			lock (_packetTimeouts)
			{
				foreach (KeyValuePair<ushort, int> pair in _packetTimeouts)
				{
					if (pair.Value != -1 && _packetTimeouts[pair.Key]-- <= 0)
					{
						server.DisconnectWithReason(server.GetSession(pair.Key), "UDP packets didnt arrive in time");
					}
				}
				return;
			}
		}
		lock (_reviveTimer)
		{
			foreach (KeyValuePair<ushort, RevivalData> i in _reviveTimer)
			{
				if (i.Value.Progress > 0.0)
				{
					i.Value.Progress -= 0.004;
					if (i.Value.Progress <= 0.0)
					{
						i.Value.DeathNote.Clear();
						server.TCPMulticast(new TcpPacket(PacketType.SERVER_REVIVAL_STATUS, false, i.Key));
					}
					server.UDPMulticast(ref IPEndPoints, new UdpPacket(PacketType.SERVER_REVIVAL_PROGRESS, i.Key, i.Value.Progress));
				}
				else
				{
					i.Value.Progress = 0.0;
				}
			}
		}
		DoTimeout(server);
		UpdateDeathTimers(server);
		_map.Tick(server);
		if (_map.Timer <= 0)
		{
			EndGame(server, 2);
		}
	}

	private void DoTimeout(Server server)
	{
		if (_timeout-- > 0)
		{
			return;
		}
		lock (server.Peers)
		{
			lock (_lastPackets)
			{
				foreach (Peer peer in server.Peers.Values)
				{
					if (!peer.Waiting && _lastPackets.ContainsKey(peer.ID))
					{
						if (peer.Player.HasEscaped || !peer.Player.IsAlive)
						{
							_lastPackets[peer.ID] = 0;
						}
						else if (Options.Get<bool>("antiafk_system"))
						{
							if (_lastPackets[peer.ID] >= 240)
							{
								server.DisconnectWithReason(server.GetSession(peer.ID), "AFK or Timeout");
							}
						}
						else
						{
							_lastPackets[peer.ID] += 60;
						}
					}
				}
			}
		}
		_timeout = 60;
	}

	private void HandlePlayers(Server server, TcpSession session, BinaryReader reader)
	{
		bool num = reader.ReadBoolean();
		byte type = reader.ReadByte();
		if (num)
		{
			server.Passtrough(reader, session);
		}
		switch ((PacketType)type)
		{
		case PacketType.IDENTITY:
			Ext.HandleIdentity(server, session, reader);
			break;
		case PacketType.CLIENT_PLAYER_DEATH_STATE:
			if (_endTimer >= 0)
			{
				break;
			}
			lock (server.Peers)
			{
				if (!server.Peers.ContainsKey(session.ID) || server.Peers[session.ID].Player.RevivalTimes >= 2)
				{
					break;
				}
				Peer peer2 = server.Peers[session.ID];
				peer2.Player.IsAlive = !reader.ReadBoolean();
				peer2.Player.RevivalTimes = reader.ReadByte();
				server.TCPMulticast(new TcpPacket(PacketType.SERVER_PLAYER_DEATH_STATE, session.ID, peer2.Player.IsAlive, (byte)peer2.Player.RevivalTimes));
				lock (_reviveTimer)
				{
					_reviveTimer[peer2.ID] = new RevivalData();
				}
				server.TCPMulticast(new TcpPacket(PacketType.SERVER_REVIVAL_STATUS, false, session.ID));
				if (!peer2.Player.IsAlive)
				{
					Terminal.LogDiscord(peer2.Nickname + " died.");
					if (peer2.Player.DiedBefore || _map.Timer <= 7200)
					{
						TcpPacket pkt = new TcpPacket(PacketType.SERVER_GAME_DEATHTIMER_END);
						if (_demonCount >= (int)((double)(server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) - 1) / 2.0))
						{
							pkt.Write(0);
						}
						else
						{
							peer2.Player.RevivalTimes = 2;
							_demonCount++;
							Terminal.LogDiscord(peer2.Nickname + " was demonized!");
							pkt.Write(1);
						}
						server.TCPSend(session, pkt);
						peer2.Player.DeadTimer = -1f;
					}
					if (peer2.Player.RevivalTimes == 0)
					{
						peer2.Player.DeadTimer = 1800f;
					}
					peer2.Player.DiedBefore = true;
				}
				else
				{
					peer2.Player.DeadTimer = -1f;
				}
				CheckEscapedAndAlive(server);
				break;
			}
		case PacketType.CLIENT_PLAYER_ESCAPED:
			if (_endTimer >= 0)
			{
				break;
			}
			lock (server.Peers)
			{
				if (server.Peers.ContainsKey(session.ID))
				{
					Peer peer = server.Peers[session.ID];
					peer.Player.HasEscaped = true;
					TcpPacket pk = new TcpPacket(PacketType.SERVER_GAME_PLAYER_ESCAPED);
					pk.Write(peer.ID);
					server.TCPMulticast(pk);
					CheckEscapedAndAlive(server);
					Terminal.LogDiscord(peer.Nickname + " has escaped!");
				}
				break;
			}
		case PacketType.CLIENT_REVIVAL_PROGRESS:
		{
			ushort rid = reader.ReadUInt16();
			byte rings = reader.ReadByte();
			lock (server.Peers)
			{
				if (server.Peers[rid].Player.IsAlive || server.Peers[rid].Player.RevivalTimes >= 2)
				{
					break;
				}
			}
			lock (_reviveTimer)
			{
				if (_reviveTimer[rid].Progress <= 0.0)
				{
					server.TCPMulticast(new TcpPacket(PacketType.SERVER_REVIVAL_STATUS, true, rid));
				}
				if (!_reviveTimer[rid].DeathNote.Contains(session.ID))
				{
					_reviveTimer[rid].DeathNote.Add(session.ID);
				}
				_reviveTimer[rid].Progress += 0.013 + 0.004 * (double)(int)rings;
				if (_reviveTimer[rid].Progress > 1.0)
				{
					foreach (ushort p in _reviveTimer[rid].DeathNote)
					{
						server.TCPSend(server.GetSession(p), new TcpPacket(PacketType.SERVER_REVIVAL_RINGSUB));
					}
					server.TCPMulticast(new TcpPacket(PacketType.SERVER_REVIVAL_STATUS, false, rid));
					server.TCPSend(server.GetSession(rid), new TcpPacket(PacketType.SERVER_REVIVAL_REVIVED));
					_reviveTimer[rid] = new RevivalData();
				}
				else
				{
					server.UDPMulticast(ref IPEndPoints, new UdpPacket(PacketType.SERVER_REVIVAL_PROGRESS, rid, _reviveTimer[rid].Progress));
				}
				break;
			}
		}
		case PacketType.CLIENT_ERECTOR_BALLS:
		{
			float x = reader.ReadSingle();
			float y = reader.ReadSingle();
			server.TCPMulticast(new TcpPacket(PacketType.CLIENT_ERECTOR_BALLS, x, y));
			break;
		}
		}
	}

	private void UpdateDeathTimers(Server server)
	{
		lock (server.Peers)
		{
			foreach (Peer peer in server.Peers.Values.OrderBy((Peer e) => e.Player.DeadTimer))
			{
				if (peer.Waiting)
				{
					continue;
				}
				if (peer.Player.IsAlive || peer.Player.HasEscaped)
				{
					peer.Player.DeadTimer = -1f;
				}
				else
				{
					if (peer.Player.DeadTimer == -1f)
					{
						continue;
					}
					if ((int)peer.Player.DeadTimer % 60 == 0)
					{
						TcpPacket pk = new TcpPacket(PacketType.SERVER_GAME_DEATHTIMER_TICK);
						pk.Write(peer.ID);
						pk.Write((byte)(peer.Player.DeadTimer / 60f));
						server.TCPMulticast(pk);
					}
					peer.Player.DeadTimer -= ((Ext.Dist(peer.Player.X, peer.Player.Y, server.Peers[_exeId].Player.X, server.Peers[_exeId].Player.Y) >= 240.0) ? 1f : 0.5f);
					if (peer.Player.DeadTimer <= 0f || _map.Timer <= 7200)
					{
						TcpPacket pkt = new TcpPacket(PacketType.SERVER_GAME_DEATHTIMER_END);
						if (_demonCount >= (int)((double)(server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) - 1) / 2.0))
						{
							pkt.Write(0);
						}
						else
						{
							_demonCount++;
							peer.Player.RevivalTimes = 2;
							Terminal.LogDiscord(peer.Nickname + " was demonized!");
							pkt.Write(1);
						}
						server.TCPSend(server.GetSession(peer.ID), pkt);
						peer.Player.DeadTimer = -1f;
					}
				}
			}
		}
	}

	private void CheckEscapedAndAlive(Server server)
	{
		lock (server.Peers)
		{
			if (_endTimer >= 0)
			{
				return;
			}
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) <= 0)
			{
				server.SetState<Lobby>();
				return;
			}
			if (!server.Peers.ContainsKey(_exeId))
			{
				EndGame(server, 1);
				return;
			}
			int alive = 0;
			int escaped = 0;
			foreach (KeyValuePair<ushort, Peer> player in server.Peers)
			{
				if (!player.Value.Waiting && player.Key != _exeId)
				{
					if (player.Value.Player.IsAlive)
					{
						alive++;
					}
					if (player.Value.Player.HasEscaped)
					{
						escaped++;
					}
				}
			}
			if (alive == 0 && escaped == 0)
			{
				EndGame(server, 0);
			}
			else if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) - alive + escaped >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting))
			{
				if (escaped == 0)
				{
					EndGame(server, 0);
				}
				else
				{
					EndGame(server, 1);
				}
			}
		}
	}

	public void EndGame(Server server, int type)
	{
		if (_endTimer < 0)
		{
			switch (type)
			{
			case 0:
			{
				_endTimer = 300;
				Terminal.LogDiscord("Exe wins!");
				TcpPacket pk = new TcpPacket(PacketType.SERVER_GAME_EXE_WINS);
				server.TCPMulticast(pk);
				break;
			}
			case 1:
			{
				_endTimer = 300;
				Terminal.LogDiscord("Survivors win!");
				TcpPacket pk2 = new TcpPacket(PacketType.SERVER_GAME_SURVIVOR_WIN);
				server.TCPMulticast(pk2);
				break;
			}
			case 2:
			{
				_endTimer = 300;
				Terminal.LogDiscord("Time over!");
				TcpPacket pk3 = new TcpPacket(PacketType.SERVER_GAME_TIME_OVER);
				server.TCPMulticast(pk3);
				TcpPacket packet = new TcpPacket(PacketType.SERVER_GAME_TIME_SYNC);
				packet.Write((ushort)0);
				server.TCPMulticast(packet);
				break;
			}
			}
		}
	}
}
