using System.Net;
using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.State;

public class Lobby : State
{
	private bool _isCounting;

	private int _countdown = 300;

	private object _cooldownLock = new object();

	private int _timeout = 60;

	private Random _rand = new Random();

	private Dictionary<ushort, int> _lastPackets = new Dictionary<ushort, int>();

	private ushort _voteKickTarget = ushort.MaxValue;

	private int _voteKickTimer;

	private List<ushort> _voteKickVotes = new List<ushort>();

	private bool _voteKick;

	private bool _practice;

	private List<ushort> _practiceVotes = new List<ushort>();

	public override DisasterServer.Session.State AsState()
	{
		return DisasterServer.Session.State.LOBBY;
	}

	public override void PeerJoined(Server server, TcpSession session, Peer peer)
	{
		if (_isCounting)
		{
			_isCounting = false;
			lock (_cooldownLock)
			{
				_countdown = 300;
			}
			MulticastState(server);
		}
		lock (_lastPackets)
		{
			_lastPackets.Add(peer.ID, 0);
		}
		peer.ExeChance = _rand.Next(2, 5);
		TcpPacket packet = new TcpPacket(PacketType.SERVER_LOBBY_EXE_CHANCE, (byte)peer.ExeChance);
		server.TCPSend(session, packet);
		packet = new TcpPacket(PacketType.SERVER_PLAYER_JOINED, peer.ID);
		server.TCPMulticast(packet, peer.ID);
	}

	public override void PeerLeft(Server server, TcpSession session, Peer peer)
	{
		lock (server.Peers)
		{
			if (server.Peers.Count <= 1)
			{
				_isCounting = false;
				lock (_cooldownLock)
				{
					_countdown = 300;
				}
				MulticastState(server);
			}
			lock (_lastPackets)
			{
				_lastPackets.Remove(peer.ID);
			}
			Terminal.LogDiscord($"{peer.Nickname} (ID {peer.ID}) left.");
		}
		lock (server.Peers)
		{
			if (server.Peers.Count <= 0)
			{
				lock (_voteKickVotes)
				{
					if (_voteKick)
					{
						Terminal.LogDiscord($"Vote kick failed for {peer.Nickname} (PID {peer.ID}) because everyone left.");
						_voteKickVotes.Clear();
						_voteKickTimer = 0;
						_voteKickTarget = ushort.MaxValue;
						_voteKick = false;
					}
				}
				lock (_practiceVotes)
				{
					if (_practice)
					{
						Terminal.LogDiscord("Practice failed because everyone left.");
						_practiceVotes.Clear();
						_practice = false;
					}
				}
			}
		}
		if (_voteKick)
		{
			lock (_voteKickVotes)
			{
				if (_voteKickVotes.Contains(session.ID))
				{
					_voteKickVotes.Remove(session.ID);
				}
				if (_voteKickTarget == session.ID)
				{
					Terminal.LogDiscord($"Vote kick failed for {peer.Nickname} (PID {peer.ID}) because player left.");
					SendMessage(server, "\\kick vote failed~ (player left)");
					_voteKickVotes.Clear();
					_voteKickTimer = 0;
					_voteKickTarget = ushort.MaxValue;
					_voteKick = false;
					return;
				}
				if (_voteKickVotes.Count >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => e.Key != _voteKickTarget))
				{
					CheckVoteKick(server, ignore: false);
				}
			}
		}
		if (!_practice)
		{
			return;
		}
		lock (_practiceVotes)
		{
			if (_practiceVotes.Contains(session.ID))
			{
				_practiceVotes.Remove(session.ID);
			}
		}
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		bool num = reader.ReadBoolean();
		byte type = reader.ReadByte();
		if (num)
		{
			server.Passtrough(reader, session);
		}
		lock (_lastPackets)
		{
			_lastPackets[session.ID] = 0;
		}
		switch ((PacketType)type)
		{
		case PacketType.IDENTITY:
			Ext.HandleIdentity(server, session, reader);
			break;
		case PacketType.CLIENT_LOBBY_PLAYERS_REQUEST:
			lock (server.Peers)
			{
				foreach (KeyValuePair<ushort, Peer> player in server.Peers)
				{
					if (!player.Value.Pending && player.Key != session.ID)
					{
						Terminal.LogDebug($"Sending {player.Value.Nickname}'s data to PID {session.ID}");
						TcpPacket pk2 = new TcpPacket(PacketType.SERVER_LOBBY_PLAYER);
						pk2.Write(player.Value.ID);
						pk2.Write(player.Value.Player.IsReady);
						pk2.Write(player.Value.Nickname.Substring(0, Math.Min(player.Value.Nickname.Length, 15)));
						pk2.Write(player.Value.Icon);
						pk2.Write(player.Value.Pet);
						server.TCPSend(session, pk2);
					}
				}
			}
			server.TCPSend(session, new TcpPacket(PacketType.SERVER_LOBBY_CORRECT));
			SendMessage(server, session, $"|- version `{Program.BUILD_VER}~");
			SendMessage(server, session, "|- edit by /miles&glitch~");
			SendMessage(server, session, "|type .help for command list~");
			break;
			/* Chat message */
            case PacketType.CLIENT_CHAT_MESSAGE:
                {
                    var id = reader.ReadUInt16();
                    var msg = reader.ReadStringNull();
                    lock (server.Peers)
                    {
                        switch (msg)
                        {
                            case ".y":
                            case ".yes":
                                lock (_voteKickVotes)
                                {
                                    if (_voteKickTimer <= 0)
                                        break;
                                    if (!_voteKickVotes.Contains(session.ID))
                                        _voteKickVotes.Add(session.ID);
                                    else
                                        break;
                                    lock (server.Peers)
                                    {
                                        SendMessage(server, $"{server.Peers[session.ID].Nickname} voted @yes~");
                                        Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} voted yes");
                                    }
                                    if (_voteKickVotes.Count >= server.Peers.Count(e => e.Key != _voteKickTarget))
                                        CheckVoteKick(server, false);
                                }
                                break;
                            case ".n":
                            case ".no":
                                lock (_voteKickVotes)
                                {
                                    if (_voteKickTimer <= 0)
                                        break;
                                    if (_voteKickVotes.Contains(session.ID))
                                        _voteKickVotes.Remove(session.ID);
                                    lock (server.Peers)
                                    {
                                        SendMessage(server, $"{server.Peers[session.ID].Nickname} voted \\no~");
                                        Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} voted no");
                                    }
                                    if (_voteKickVotes.Count >= server.Peers.Count(e => e.Key != _voteKickTarget))
                                        CheckVoteKick(server, false);
                                }
                                break;
                            case ".help":
                            case ".h":
                                SendMessage(server, session, $"~----------------------");
                                SendMessage(server, session, "|list of commands:~");
                                SendMessage(server, session, "@.practice~ (.p) - practice mode vote");
                                SendMessage(server, session, "@.mute~ (.m) - toggle chat messages");
                                SendMessage(server, session, "@.votekick~ (.vk) - kick vote a player");
                                SendMessage(server, session, $"~----------------------");
                                break;
                            case ".practice":
                            case ".p":
                                if (_practice)
                                {
                                    lock (_practiceVotes)
                                    {
                                        if (_practiceVotes.Contains(session.ID))
                                            break;
                                        lock (server.Peers)
                                        {
                                            SendMessage(server, $"{server.Peers[session.ID].Nickname} wants to `practice~");
                                            Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} wants to practice");
                                            _practiceVotes.Add(session.ID);
                                            if (_practiceVotes.Count >= server.Peers.Count)
                                                server.SetState(new CharacterSelect(new FartZone()));
                                        }
                                    }
                                    break;
                                }
                                lock (_practiceVotes)
                                {
                                    lock (server.Peers)
                                    {
                                        SendMessage(server, $"{server.Peers[session.ID].Nickname} wants to `practice~");
                                        Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} wants to practice");
                                        _practiceVotes.Add(session.ID);
                                    }
                                }
                                Terminal.LogDiscord($"{server.Peers[session.ID].Nickname} started practice vote");
                                SendMessage(server, $"~----------------------");
                                SendMessage(server, $"\\`practice~ vote started by /{server.Peers[id].Nickname}~");
                                SendMessage(server, $"type `.p~ for practice room");
                                SendMessage(server, $"~----------------------");
                                _practice = true;
                                break;
                            default:
                                foreach (var peer in server.Peers.Values)
                                {
                                    if (peer.Waiting)
                                        continue;
                                    if (peer.ID != id)
                                        continue;
                                    Terminal.LogDiscord($"[{peer.Nickname}]: {msg}");
                                }
                                break;
                        }
                    }
                    break; 
                }
		case PacketType.CLIENT_LOBBY_READY_STATE:
		{
			bool ready = reader.ReadBoolean();
			lock (server.Peers)
			{
				if (server.Peers.ContainsKey(session.ID))
				{
					Peer peer = server.Peers[session.ID];
					peer.Player.IsReady = ready;
					TcpPacket pk = new TcpPacket(PacketType.SERVER_LOBBY_READY_STATE);
					pk.Write(peer.ID);
					pk.Write(ready);
					server.TCPMulticast(pk, session.ID);
					CheckReadyPeers(server);
				}
				break;
			}
		}
		case PacketType.CLIENT_LOBBY_VOTEKICK:
		{
			ushort id = reader.ReadUInt16();
			lock (server.Peers)
			{
				lock (_voteKickVotes)
				{
					if (id == _voteKickTarget)
					{
						if (!_voteKickVotes.Contains(session.ID))
						{
							_voteKickVotes.Add(session.ID);
							SendMessage(server, server.Peers[session.ID].Nickname + " voted @yes~");
							if (_voteKickVotes.Count >= server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => e.Key != _voteKickTarget))
							{
								CheckVoteKick(server, ignore: false);
							}
						}
					}
					else if (_voteKick)
					{
						SendMessage(server, session, "\\kick vote is already in process!");
					}
					else
					{
						VoteKickStart(server, session.ID, id);
						SendMessage(server, server.Peers[session.ID].Nickname + " voted @yes~");
					}
					break;
				}
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
			foreach (Peer peer in server.Peers.Values)
			{
				if (peer.Waiting)
				{
					TcpPacket pak = new TcpPacket(PacketType.SERVER_IDENTITY_RESPONSE);
					pak.Write(value: true);
					pak.Write(peer.ID);
					server.TCPSend(server.GetSession(peer.ID), pak);
					peer.Waiting = false;
				}
				else
				{
					TcpPacket pk = new TcpPacket(PacketType.SERVER_GAME_BACK_TO_LOBBY);
					server.TCPSend(server.GetSession(peer.ID), pk);
				}
				lock (_lastPackets)
				{
					_lastPackets.Add(peer.ID, 0);
				}
				peer.Player = new Player();
				if (peer.ExeChance >= 99)
				{
					peer.ExeChance = 99;
				}
				TcpPacket packet = new TcpPacket(PacketType.SERVER_LOBBY_EXE_CHANCE, (byte)peer.ExeChance);
				server.TCPSend(server.GetSession(peer.ID), packet);
			}
		}
		Program.Stat?.MulticastInformation();
	}

	public override void Tick(Server server)
	{
		if (_isCounting)
		{
			lock (_cooldownLock)
			{
				_countdown--;
				if (_countdown <= 0)
				{
					CheckVoteKick(server, ignore: true);
					server.SetState<MapVote>();
				}
				else if (_countdown % 60 == 0)
				{
					MulticastState(server);
				}
			}
		}
		DoVoteKick(server);
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
			lock (_lastPackets)
			{
				foreach (Peer peer in server.Peers.Values)
				{
					if (peer.Waiting)
					{
						_lastPackets[peer.ID] = 0;
					}
					else if (_lastPackets.ContainsKey(peer.ID))
					{
						if (peer.Player.IsReady)
						{
							_lastPackets[peer.ID] = 0;
						}
						else if (_lastPackets[peer.ID] >= 1500)
						{
							server.DisconnectWithReason(server.GetSession(peer.ID), "AFK or Timeout");
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

	private void CheckReadyPeers(Server server)
	{
		lock (server.Peers)
		{
			int totalReady = 0;
			foreach (Peer value in server.Peers.Values)
			{
				if (value.Player.IsReady)
				{
					totalReady++;
				}
			}
			if (totalReady >= server.Peers.Count && totalReady > 1)
			{
				_isCounting = true;
				MulticastState(server);
			}
			else if (_isCounting)
			{
				_isCounting = false;
				lock (_cooldownLock)
				{
					_countdown = 300;
				}
				MulticastState(server);
			}
		}
	}

	private void MulticastState(Server server)
	{
		TcpPacket packet = new TcpPacket(PacketType.SERVER_LOBBY_COUNTDOWN, _isCounting, (byte)(_countdown / 60));
		server.TCPMulticast(packet);
	}

	private void DoVoteKick(Server server)
	{
		lock (_voteKickVotes)
		{
			if (_voteKickTimer > 0)
			{
				_voteKickTimer--;
				if (_voteKickTimer <= 0)
				{
					CheckVoteKick(server, ignore: true);
				}
			}
		}
	}

	private void CheckVoteKick(Server server, bool ignore)
	{
		if ((_voteKickTimer <= 0 && !ignore) || !_voteKick)
		{
			return;
		}
		int totalFor = _voteKickVotes.Count;
		int totalAgainst = 0;
		lock (server.Peers)
		{
			if (!server.Peers.ContainsKey(_voteKickTarget))
			{
				Terminal.LogDiscord($"Vote kick failed for PID {_voteKickTarget} because player left.");
				SendMessage(server, "\\kick vote failed~ (player left)");
				_voteKickVotes.Clear();
				_voteKickTimer = 0;
				_voteKickTarget = ushort.MaxValue;
				_voteKick = false;
				return;
			}
			foreach (KeyValuePair<ushort, Peer> peer in server.Peers)
			{
				if (peer.Value.Waiting)
				{
					continue;
				}
				bool has = false;
				foreach (ushort peer2 in _voteKickVotes)
				{
					if (peer.Key == peer2)
					{
						has = true;
						break;
					}
				}
				if (!has)
				{
					totalAgainst++;
				}
			}
		}
		if (totalFor >= totalAgainst)
		{
			Terminal.LogDiscord($"Vote kick succeeded for {server.Peers[_voteKickTarget].Nickname} (PID {_voteKickTarget})");
			SharedServerSession session = server.GetSession(_voteKickTarget);
			KickList.Add((session.RemoteEndPoint as IPEndPoint).Address.ToString());
			server.DisconnectWithReason(session, "Vote kick.");
			SendMessage(server, $"@kick vote success~ (@{totalFor}~ vs \\{totalAgainst}~)");
			_voteKickVotes.Clear();
			_voteKickTimer = 0;
			_voteKickTarget = ushort.MaxValue;
			_voteKick = false;
		}
		else
		{
			Terminal.LogDiscord($"Vote kick failed for {server.Peers[_voteKickTarget].Nickname} (PID {_voteKickTarget})");
			SendMessage(server, $"\\kick vote failed~ (@{totalFor}~ vs \\{totalAgainst}~)");
			_voteKickVotes.Clear();
			_voteKickTimer = 0;
			_voteKickTarget = ushort.MaxValue;
			_voteKick = false;
		}
	}

	private void VoteKickStart(Server server, ushort voter, ushort id)
	{
		_voteKickTarget = id;
		_voteKickVotes.Clear();
		_voteKickVotes.Add(voter);
		_voteKickTimer = 900;
		_voteKick = true;
		SendMessage(server, "~----------------------");
		SendMessage(server, "\\kick vote started for /" + server.Peers[id].Nickname + "~");
		SendMessage(server, "type @.y~ or \\.n~");
		SendMessage(server, "~----------------------");
		Terminal.LogDiscord($"Vote kick started for {server.Peers[id].Nickname} (PID {id})");
	}

	private void SendMessage(Server server, string text)
	{
		TcpPacket pack = new TcpPacket(PacketType.CLIENT_CHAT_MESSAGE, (ushort)0);
		pack.Write(text);
		server.TCPMulticast(pack);
	}

	private void SendMessage(Server server, ushort id, string text)
	{
		TcpPacket pack = new TcpPacket(PacketType.CLIENT_CHAT_MESSAGE, (ushort)0);
		pack.Write(text);
		server.TCPSend(server.GetSession(id), pack);
	}

	private void SendMessage(Server server, TcpSession session, string text)
	{
		TcpPacket pack = new TcpPacket(PacketType.CLIENT_CHAT_MESSAGE, (ushort)0);
		pack.Write(text);
		server.TCPSend(session, pack);
	}
}
