using System;
using System.Collections.Generic;
using System.IO;

namespace Server.Game;

internal class ServerSession : ServerSessionBase
{
	public ServerSession(Server server)
		: base(server)
	{
		_server = server;
	}

	protected override void OnConnected()
	{
		if (_server.Players.Count >= 7)
		{
			Packet p = new Packet(PacketType.SERVER_PLAYER_FORCE_DISCONNECT);
			p.Write("Game is full!");
			SendAsync(p);
			return;
		}
		if (_server.State != 0)
		{
			Packet p2 = new Packet(PacketType.SERVER_PLAYER_FORCE_DISCONNECT);
			p2.Write("Game has already started!");
			SendAsync(p2);
			return;
		}
		lock (_server.Countdown)
		{
			if (_server.Countdown.IsCounting)
			{
				_server.Countdown.Count = 5;
				_server.Countdown.Stop();
				Packet packet2 = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
				packet2.Write((byte)0);
				packet2.Write((byte)5);
				_server.MulticastAsync(packet2);
			}
		}
		ushort id = ++_server.IdCounter;
		lock (_server.Players)
		{
			_server.Players.TryAdd(base.Id, new Player
			{
				GID = base.Id,
				ID = id,
				EndPoint = base.Socket.RemoteEndPoint
			});
			Packet packet = new Packet(PacketType.SERVER_PLAYER_JOINED);
			packet.Write(id);
			_server.MulticastAsync(packet, base.Id);
			packet = new Packet(PacketType.SERVER_REQUEST_INFO);
			packet.Write(id);
			SendAsync(packet);
		}
		base.OnConnected();
	}

	protected override void OnDisconnected()
	{
		lock (_server.Players)
		{
			if (!_server.Players.ContainsKey(base.Id))
			{
				return;
			}
			_server.Players.Remove(base.Id, out var plr);
			if (plr == null)
			{
				return;
			}
			switch (_server.State)
			{
			case State.VOTE:
				_server.CheckLeftPlayers();
				break;
			case State.GAME:
				_server.CheckEscapedAndAlive();
				break;
			}
			if (_server.State == State.LOBBY && _server.Players.Count == 1 && _server.Countdown.IsCounting)
			{
				_server.Countdown.Count = 5;
				_server.Countdown.Stop();
				Packet pk = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
				pk.Write((byte)0);
				pk.Write((byte)5);
				_server.MulticastAsync(pk);
			}
			Logger.Log(plr.Nickname + " left.", ConsoleColor.White, _server.ID);
			Packet packet = new Packet(PacketType.SERVER_PLAYER_LEFT);
			packet.Write(plr.ID);
			_server.MulticastAsync(packet, base.Id);
		}
		base.OnDisconnected();
	}

	protected override void PacketReady(byte[] data)
	{
		using (MemoryStream ms = new MemoryStream(data))
		{
			using BinaryReader rd = new BinaryReader(ms);
			if (rd.ReadBoolean())
			{
				Packet packet3 = new Packet();
				for (int i = 0; i < data.Length; i++)
				{
					packet3.Write(data[i]);
				}
				_server.MulticastAsync(packet3, base.Id);
				return;
			}
			switch ((PacketType)rd.ReadByte())
			{
			case PacketType.CLIENT_REQUESTED_INFO:
			{
				if (_server.State != 0)
				{
					break;
				}
				ushort ver = rd.ReadUInt16();
				if (ver != 206)
				{
					Packet pk3 = new Packet(PacketType.SERVER_PLAYER_FORCE_DISCONNECT);
					pk3.Write($"Wrong game version ({206} required, but got {ver})");
					SendAsync(pk3);
					lock (_server.Players)
					{
						_server.Players.Remove(base.Id, out var _);
					}
				}
				Packet packet5 = new Packet(PacketType.SERVER_PLAYER_INFO);
				lock (_server.Players)
				{
					string name = rd.ReadStringNull();
					if (_server.Players.ContainsKey(base.Id))
					{
						_server.Players[base.Id].LastPacketTime = 0;
						_server.Players[base.Id].Pending = false;
						_server.Players[base.Id].Character = Character.NONE;
						_server.Players[base.Id].Nickname = name;
						_server.Players[base.Id].RevivalTimes = 0;
						_server.Players[base.Id].IsAlive = true;
						_server.Players[base.Id].HasEscaped = false;
						_server.Players[base.Id].IsReady = false;
						Logger.Log(_server.Players[base.Id].Nickname + " joined.", ConsoleColor.White, _server.ID);
						packet5.Write(_server.Players[base.Id].ID);
						packet5.Write(name);
					}
				}
				_server.MulticastAsync(packet5, base.Id);
				break;
			}
			case PacketType.CLIENT_LOBBY_PLAYERS_REQUEST:
				if (_server.State != 0)
				{
					break;
				}
				lock (_server.Players)
				{
					foreach (KeyValuePair<Guid, Player> player in _server.Players)
					{
						if (!player.Value.Pending && !(player.Key == base.Id))
						{
							Packet packet4 = new Packet(PacketType.SERVER_LOBBY_PLAYER);
							packet4.Write(player.Value.ID);
							packet4.Write(player.Value.IsReady);
							packet4.Write(player.Value.Nickname);
							SendAsync(packet4);
						}
					}
				}
				break;
			case PacketType.CLIENT_LOBBY_READY_STATE:
			{
				if (_server.State != 0)
				{
					break;
				}
				bool ready = rd.ReadBoolean();
				lock (_server.Players)
				{
					_server.Players[base.Id].IsReady = ready;
					int cnt = 0;
					foreach (KeyValuePair<Guid, Player> player3 in _server.Players)
					{
						if (player3.Value.IsReady)
						{
							cnt++;
						}
					}
					lock (_server.Countdown)
					{
						if (cnt >= _server.Players.Count && _server.Players.Count > 1 && !_server.Countdown.IsCounting)
						{
							_server.Countdown.Count = 5;
							_server.Countdown.Start();
							Packet packet2 = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
							packet2.Write((byte)1);
							packet2.Write((byte)5);
							_server.MulticastAsync(packet2);
						}
						else if (_server.Countdown.IsCounting)
						{
							_server.Countdown.Count = 5;
							_server.Countdown.Stop();
							Packet packet = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
							packet.Write((byte)0);
							packet.Write((byte)5);
							_server.MulticastAsync(packet);
						}
					}
					Packet pk = new Packet(PacketType.SERVER_LOBBY_READY_STATE);
					pk.Write(_server.Players[base.Id].ID);
					pk.Write(ready);
					_server.MulticastAsync(pk, base.Id);
				}
				break;
			}
			case PacketType.CLIENT_REQUEST_CHARACTER:
			{
				if (_server.State != State.VOTE)
				{
					break;
				}
				byte id = rd.ReadByte();
				bool canUse = true;
				int cnt2 = 0;
				lock (_server.Players)
				{
					foreach (KeyValuePair<Guid, Player> player2 in _server.Players)
					{
						if (player2.Value.Character == (Character)id)
						{
							canUse = false;
						}
						if (player2.Value.Character != Character.NONE)
						{
							cnt2++;
						}
					}
					if (canUse)
					{
						_server.Players[base.Id].Character = (Character)id;
						Logger.Log($"{_server.Players[base.Id].Nickname} chooses {id}", ConsoleColor.Yellow, _server.ID);
						Packet packet7 = new Packet(PacketType.SERVER_LOBBY_CHARACTER_RESPONSE);
						packet7.Write(id);
						packet7.Write(value: true);
						SendAsync(packet7);
						packet7 = new Packet(PacketType.SERVER_LOBBY_CHARACTER_CHANGE);
						packet7.Write(_server.Players[base.Id].ID);
						packet7.Write(id);
						_server.MulticastAsync(packet7, base.Id);
						if (cnt2 >= _server.Players.Count - 1)
						{
							_server.StartGame();
						}
					}
					else
					{
						Packet packet6 = new Packet(PacketType.SERVER_LOBBY_CHARACTER_RESPONSE);
						packet6.Write(id);
						packet6.Write(value: false);
						SendAsync(packet6);
					}
				}
				break;
			}
			case PacketType.CLIENT_PLAYER_DEATH_STATE:
				if (_server.GameEnded)
				{
					break;
				}
				lock (_server.Players)
				{
					if (!_server.Players.ContainsKey(base.Id))
					{
						break;
					}
					_server.Players[base.Id].IsAlive = !rd.ReadBoolean();
					_server.Players[base.Id].RevivalTimes = rd.ReadByte();
					if (!_server.Players[base.Id].IsAlive)
					{
						Logger.Log(_server.Players[base.Id].Nickname + " died.", ConsoleColor.Red, _server.ID);
						if (_server.Players[base.Id].DiedBefore)
						{
							Packet pkt = new Packet(PacketType.SERVER_GAME_DEATHTIMER_END);
							if (_server.DemonCount >= (int)((double)(_server.Players.Count - 1) / 2.0))
							{
								pkt.Write(0);
							}
							else
							{
								_server.Players[base.Id].RevivalTimes = 2;
								_server.DemonCount++;
								Logger.Log(_server.Players[base.Id].Nickname + " was demonized!", ConsoleColor.Red, _server.ID);
								pkt.Write(1);
							}
							SendAsync(pkt);
							_server.Players[base.Id].DeadTimer = -1;
						}
						if (_server.Players[base.Id].RevivalTimes == 0)
						{
							_server.Players[base.Id].DeadTimer = 30;
						}
						_server.Players[base.Id].DiedBefore = true;
					}
					else
					{
						_server.Players[base.Id].DeadTimer = -1;
					}
					goto IL_0a13;
				}
			case PacketType.CLIENT_PLAYER_ESCAPED:
				{
					if (_server.GameEnded)
					{
						break;
					}
					lock (_server.Players)
					{
						if (!_server.Players.ContainsKey(base.Id))
						{
							break;
						}
						Logger.Log(_server.Players[base.Id].Nickname + " has escaped!", ConsoleColor.Green, _server.ID);
						Packet pk2 = new Packet(PacketType.SERVER_GAME_PLAYER_ESCAPED);
						pk2.Write(_server.Players[base.Id].ID);
						_server.MulticastAsync(pk2);
						_server.Players[base.Id].HasEscaped = true;
						goto IL_0b0e;
					}
				}
				IL_0b0e:
				_server.CheckEscapedAndAlive();
				break;
				IL_0a13:
				_server.CheckEscapedAndAlive();
				break;
			}
		}
		base.PacketReady(data);
	}
}
