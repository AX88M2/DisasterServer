using System.Net;
using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.State;

internal class CharacterSelect : State
{
	private Random _rand = new Random();

	private int _timeout;

	private Peer _exe;

	private Map _map;

	private Dictionary<ushort, int> _lastPackets = new Dictionary<ushort, int>();

	public CharacterSelect(Map map)
	{
		_map = map;
	}

	public override DisasterServer.Session.State AsState()
	{
		return DisasterServer.Session.State.CHARACTERSELECT;
	}

	public override void PeerJoined(Server server, TcpSession session, Peer peer)
	{
	}

	public override void PeerLeft(Server server, TcpSession session, Peer peer)
	{
		lock (server.Peers)
		{
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) <= 1 || _exe == peer)
			{
				server.SetState<Lobby>();
				return;
			}
			int cnt = 0;
			foreach (Peer peer1 in server.Peers.Values)
			{
				if (!peer1.Waiting && peer1.Player.Character != Character.None && (peer1.Player.Character != 0 || peer1.Player.ExeCharacter != ExeCharacter.None))
				{
					cnt++;
				}
			}
			if (cnt >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting))
			{
				server.SetState(new Game(_map, _exe.ID));
			}
		}
		lock (_lastPackets)
		{
			_lastPackets.Remove(peer.ID);
		}
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		lock (_lastPackets)
		{
			_lastPackets[session.ID] = 0;
		}
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
		case PacketType.CLIENT_REQUEST_EXECHARACTER:
		{
			int id2 = reader.ReadByte() - 1;
			lock (server.Peers)
			{
				if (server.Peers[session.ID].Player.Character != 0 || server.Peers[session.ID].Player.ExeCharacter != ExeCharacter.None)
				{
					break;
				}
				int cnt2 = 0;
				foreach (Peer peer3 in server.Peers.Values)
				{
					if (!peer3.Waiting && peer3.Player.Character != Character.None && (peer3.Player.Character != 0 || peer3.Player.ExeCharacter != ExeCharacter.None))
					{
						cnt2++;
					}
				}
				server.Peers[session.ID].Player.ExeCharacter = (ExeCharacter)id2;
				Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} chooses {id2}");
				TcpPacket packet3 = new TcpPacket(PacketType.SERVER_LOBBY_EXECHARACTER_RESPONSE, id2);
				server.TCPSend(session, packet3);
				packet3 = new TcpPacket(PacketType.SERVER_LOBBY_CHARACTER_CHANGE);
				packet3.Write(session.ID);
				packet3.Write(id2);
				server.TCPMulticast(packet3, session.ID);
				if (++cnt2 >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting))
				{
					server.SetState(new Game(_map, _exe.ID));
				}
				break;
			}
		}
		case PacketType.CLIENT_REQUEST_CHARACTER:
		{
			byte id = reader.ReadByte();
			bool canUse = true;
			int cnt = 0;
			lock (server.Peers)
			{
				if (server.Peers[session.ID].Player.Character != Character.None)
				{
					break;
				}
				foreach (Peer peer in server.Peers.Values)
				{
					if (peer.Waiting)
					{
						continue;
					}

					if (peer.Player.Character == (Character)id)
					{
						canUse = false;
					}

					if (peer.Player.Character != Character.None)
					{
						if (peer.Player.Character == Character.Exe && peer.Player.ExeCharacter == ExeCharacter.None){
							continue;
						}

						cnt++;
					}
				}
				if (canUse)
				{
					if (!server.Peers.ContainsKey(session.ID))
					{
						break;
					}

					var peer = server.Peers[session.ID];
					peer.Player.Character = (Character)id;

					var packet = new TcpPacket(PacketType.SERVER_LOBBY_CHARACTER_RESPONSE, id, true);
					server.TCPSend(session, packet);

					packet = new TcpPacket(PacketType.SERVER_LOBBY_CHARACTER_CHANGE);
					packet.Write(session.ID);
					packet.Write(id);
					server.TCPMulticast(packet, session.ID);

					Terminal.LogDiscord($"{peer.Nickname} chooses {(Character)id}");

					if (++cnt >= server.Peers.Count(e => !e.Value.Waiting))
					{
						server.SetState(new Game(_map, _exe.ID));
					}
				}
				else
				{
					var packet = new TcpPacket(PacketType.SERVER_LOBBY_CHARACTER_RESPONSE, id, false);
					server.TCPSend(session, packet);
				}
				break;
			}
		}
		}
	}

	public override void PeerUDPMessage(Server server, IPEndPoint IPEndPoint, ref byte[] data)
	{
	}

	public override void Init(Server server)
	{
		lock (server.Peers)
		{
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) <= 1 && !(_map is FartZone))
			{
				server.SetState<Lobby>();
			}
			int ind = 0;
			foreach (Peer peer2 in server.Peers.Values)
			{
				if (peer2.Pending)
				{
					server.DisconnectWithReason(server.GetSession(peer2.ID), "PacketType.CLIENT_REQUESTED_INFO missing.");
				}
				else if (!peer2.Waiting)
				{
					lock (_lastPackets)
					{
						_lastPackets.Add(peer2.ID, 0);
					}
					peer2.Player = new Player();
					ind++;
				}
			}
			_exe = ChooseExe(server) ?? server.Peers[0];
			_exe.Player.Character = Character.Exe;
			_exe.ExeChance = 0;
			foreach (Peer peer in server.Peers.Values)
			{
				if (!peer.Waiting)
				{
					//Вычесления кто будет exe
					if (peer.Player.Character != 0)
					{
						peer.ExeChance += _rand.Next(4, 10);
					}
					else
					{
						peer.ExeChance += _rand.Next(0, 2);
					}
				}
			}
			Terminal.LogDiscord($"Map is {_map}");
			TcpPacket packet = new TcpPacket(PacketType.SERVER_LOBBY_EXE);
			packet.Write(_exe.ID);
			packet.Write((ushort)Array.IndexOf(MapVote.Maps, _map?.GetType()));
			server.TCPMulticast(packet);
		}
		Program.Stat?.MulticastInformation();
	}

	private Peer? ChooseExe(Server server)
	{
		Dictionary<ushort, double> chances = new Dictionary<ushort, double>();
		double accWeight = 0.0;
		lock (server.Peers)
		{
			foreach (Peer peer in server.Peers.Values)
			{
				if (!peer.Waiting)
				{
					accWeight += (double)peer.ExeChance;
					_ = peer.ExeChance;
					chances.Add(peer.ID, accWeight);
				}
			}
			double r = _rand.NextDouble() * accWeight;
			foreach (KeyValuePair<ushort, double> chance in chances)
			{
				if (chance.Value >= r)
				{
					return server.Peers.Values.FirstOrDefault((Peer e) => e.ID == chance.Key && !e.Waiting);
				}
			}
			return server.Peers.Values.FirstOrDefault();
		}
	}

	public override void Tick(Server server)
	{
		DoTimeout(server);
	}

	private void DoTimeout(Server server)
	{
		if (_timeout-- > 0)
		{
			return;
		}
		lock (server.Peers)
		{
			foreach (Peer peer in server.Peers.Values)
			{
				if (peer.Waiting || !_lastPackets.Any((KeyValuePair<ushort, int> e) => e.Key == peer.ID))
				{
					continue;
				}
				lock (_lastPackets)
				{
					if (peer.Player.Character != Character.None && peer.Player.Character != 0)
					{
						_lastPackets[peer.ID] = 0;
						continue;
					}
					if (peer.Player.Character == Character.Exe && peer.Player.ExeCharacter != ExeCharacter.None)
					{
						_lastPackets[peer.ID] = 0;
						continue;
					}
					if (Options.Get<bool>("antiafk_system") && _lastPackets[peer.ID] >= 1800)
					{
						server.DisconnectWithReason(server.GetSession(peer.ID), "AFK or Timeout");
						continue;
					}
					server.TCPSend(server.GetSession(peer.ID), new TcpPacket(PacketType.SERVER_CHAR_TIME_SYNC, (byte)(30 - _lastPackets[peer.ID] / 60)));
					_lastPackets[peer.ID] += 60;
				}
			}
		}
		_timeout = 60;
	}
}
