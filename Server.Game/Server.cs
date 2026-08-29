using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Timers;
using NetCoreServer;

namespace Server.Game;

public class Server : TcpServer
{
	public ConcurrentDictionary<Guid, Player> Players = new ConcurrentDictionary<Guid, Player>();

	public Countdown Countdown = new Countdown();

	public Random Rand = new Random();

	public State State;

	public ushort IdCounter;

	public int ID;

	public const int BUILD_VER = 206;

	public static readonly string[] MAPS = new string[3] { "Hide and Seek Act 2", "Ravine Mist", "..." };

	public Guid ExeId = Guid.Empty;

	public bool GameEnded;

	public int DemonCount;

	private int _ringTimer;

	private int _bigRingTimer;

	private int _totalTimer;

	private Timer _timer = new Timer(1000.0);

	public Server(int id, int port)
		: base(IPAddress.Any, port)
	{
		ID = id;
		base.OptionNoDelay = true;
		base.OptionReceiveBufferSize = 16;
		base.OptionSendBufferSize = 16;
		base.OptionKeepAlive = true;
		base.OptionTcpKeepAliveInterval = 2;
		base.OptionTcpKeepAliveTime = 5;
		base.OptionTcpKeepAliveRetryCount = 2;
		_timer.Elapsed += HeartbeatTick;
		_timer.Start();
		Countdown.CountEvent += Countdown_Count;
		Countdown.ReachedEvent += Countdown_Reached;
	}

	public void CheckEscapedAndAlive()
	{
		lock (Players)
		{
			if (GameEnded)
			{
				return;
			}
			if (Players.Count <= 0)
			{
				StartLobby();
				return;
			}
			if (!Players.ContainsKey(ExeId))
			{
				Logger.Log("Survivors win!", ConsoleColor.Green, ID);
				Packet packet = new Packet(PacketType.SERVER_GAME_SURVIVOR_WIN);
				MulticastAsync(packet);
				EndGame();
				return;
			}
			int alive = 0;
			int escaped = 0;
			foreach (KeyValuePair<Guid, Player> player in Players)
			{
				if (!(player.Key == ExeId))
				{
					if (player.Value.IsAlive)
					{
						alive++;
					}
					if (player.Value.HasEscaped)
					{
						escaped++;
					}
				}
			}
			if (alive == 0)
			{
				Logger.Log("Exe wins!", ConsoleColor.Red, ID);
				Packet packet4 = new Packet(PacketType.SERVER_GAME_EXE_WINS);
				MulticastAsync(packet4);
				EndGame();
			}
			else if (Players.Count - alive + escaped >= Players.Count)
			{
				if (escaped == 0)
				{
					Logger.Log("Exe wins!", ConsoleColor.Red, ID);
					Packet packet3 = new Packet(PacketType.SERVER_GAME_EXE_WINS);
					MulticastAsync(packet3);
					EndGame();
				}
				else
				{
					Logger.Log("Survivors win!", ConsoleColor.Green, ID);
					Packet packet2 = new Packet(PacketType.SERVER_GAME_SURVIVOR_WIN);
					MulticastAsync(packet2);
					EndGame();
				}
			}
		}
	}

	public void CheckLeftPlayers()
	{
		if (Players.Count <= 1)
		{
			Packet pak = new Packet(PacketType.SERVER_GAME_BACK_TO_LOBBY);
			MulticastAsync(pak);
			StartLobby();
			return;
		}
		if (base.Id == ExeId)
		{
			lock (Players)
			{
				ExeId = Players.ElementAt(Rand.Next(Players.Count)).Key;
				Players[ExeId].Character = Character.EXE;
				Packet pak2 = new Packet(PacketType.SERVER_LOBBY_EXE_CHANGE);
				pak2.Write(Players[ExeId].ID);
				MulticastAsync(pak2);
				return;
			}
		}
		lock (Players)
		{
			int cnt = 0;
			foreach (KeyValuePair<Guid, Player> player in Players)
			{
				if (player.Value.Character != Character.NONE)
				{
					cnt++;
				}
			}
			if (cnt >= Players.Count - 1)
			{
				StartGame();
			}
		}
	}

	public void StartGame()
	{
		State = State.GAME;
		DemonCount = 0;
		_bigRingTimer = 0;
		_ringTimer = 0;
		_totalTimer = 180 + (Players.Count - 1) * 20;
		lock (Countdown)
		{
			Countdown.Count = _totalTimer;
			Countdown.Start();
		}
		lock (Players)
		{
			foreach (KeyValuePair<Guid, Player> plr in Players)
			{
				plr.Value.DiedBefore = false;
				plr.Value.IsAlive = true;
				plr.Value.LastPacketTime = 0;
			}
		}
		Logger.Log($"Game started! (Time {Countdown.Count}s)", ConsoleColor.White, ID);
		Packet packet = new Packet(PacketType.SERVER_LOBBY_GAME_START);
		MulticastAsync(packet);
	}

	public void StartLobby()
	{
		Countdown.Stop();
		lock (Players)
		{
			foreach (KeyValuePair<Guid, Player> player in Players)
			{
				player.Value.Character = Character.NONE;
				player.Value.IsReady = false;
				player.Value.DeadTimer = -1;
				player.Value.HasEscaped = false;
				player.Value.IsAlive = true;
				player.Value.RevivalTimes = 0;
				player.Value.LastPacketTime = 0;
			}
		}
		GameEnded = false;
		ExeId = Guid.Empty;
		State = State.LOBBY;
	}

	private void EndGame()
	{
		GameEnded = true;
		Countdown.Count = 5;
		Countdown.Start();
	}

	private void HeartbeatTick(object? sender, ElapsedEventArgs e)
	{
		lock (Players)
		{
			foreach (KeyValuePair<Guid, Player> p in Players)
			{
				if (State != State.GAME && (Countdown.IsCounting || p.Value.IsReady || p.Value.Character != Character.NONE))
				{
					p.Value.LastPacketTime = 0;
					continue;
				}
				if (State == State.GAME && p.Value.HasEscaped)
				{
					p.Value.LastPacketTime = 0;
					continue;
				}
				//Anti-AFK System
				if (Program.enabledAntiAfkSystem) {
					Logger.Log("[Anti-AFK] "+$"{p.Key}: {p.Value.LastPacketTime}",ConsoleColor.Magenta);
                    //Console.WriteLine($"{p.Key}: {p.Value.LastPacketTime}");
                    if (p.Value.LastPacketTime > ((State == State.GAME) ? 20 : 30))
                    {
                        Logger.Log(p.Value.Nickname + " disconnected for AFK.", ConsoleColor.Red, ID);
                        Packet pk = new Packet(PacketType.SERVER_PLAYER_FORCE_DISCONNECT);
                        pk.Write("AFK/Conection issue");
                        TcpSession tcpSession = FindSession(p.Key);
                        tcpSession.Send(pk.ToArray());
                        tcpSession.Disconnect();
                        CheckLeftPlayers();
                    }
                }
				p.Value.LastPacketTime++;
			}
		}
		Packet packet = new Packet(PacketType.SERVER_HEARTBEAT);
		MulticastAsync(packet);
	}

	private void StartVote()
	{
		lock (Players)
		{
			int map = Rand.Next(MAPS.Length);
			if (Players.Count <= 0)
			{
				Packet pk = new Packet(PacketType.SERVER_GAME_BACK_TO_LOBBY);
				MulticastAsync(pk);
				StartLobby();
				return;
			}
			ExeId = Players.ElementAt(Rand.Next(Players.Count)).Key;
			foreach (KeyValuePair<Guid, Player> i in Players)
			{
				i.Value.Character = Character.NONE;
				i.Value.IsReady = false;
				i.Value.LastPacketTime = 0;
			}
			Players[ExeId].Character = Character.EXE;
			Logger.Log(Players[ExeId].Nickname + " is EXE!", ConsoleColor.Red, ID);
			Logger.Log("Map is " + MAPS[map], ConsoleColor.White, ID);
			Packet packet = new Packet(PacketType.SERVER_LOBBY_EXE);
			packet.Write(Players[ExeId].ID);
			packet.Write((ushort)map);
			MulticastAsync(packet);
		}
		State = State.VOTE;
	}

	private void GameTick()
	{
		_ringTimer++;
		if (_bigRingTimer != -1)
		{
			_bigRingTimer++;
		}
		if (_ringTimer > 3)
		{
			Packet packet = new Packet(PacketType.SERVER_RING_SPAWNED);
			packet.Write((byte)Rand.Next(255));
			packet.Write(Rand.Next(100) <= 10);
			MulticastAsync(packet);
			_ringTimer = 0;
		}
		if (_bigRingTimer > _totalTimer - 60)
		{
			Logger.Log("Escape ring spawned!", ConsoleColor.Green, ID);
			Packet packet2 = new Packet(PacketType.SERVER_GAME_SPAWN_RING);
			packet2.Write((byte)Rand.Next(255));
			MulticastAsync(packet2);
			_bigRingTimer = -1;
		}
		lock (Players)
		{
			foreach (KeyValuePair<Guid, Player> plr in Players)
			{
				if (plr.Value.IsAlive || plr.Value.RevivalTimes >= 2)
				{
					plr.Value.DeadTimer = -1;
				}
				else
				{
					if (plr.Value.DeadTimer == -1)
					{
						continue;
					}
					if (Countdown.Count > 120)
					{
						plr.Value.DeadTimer--;
					}
					else
					{
						plr.Value.DeadTimer = 0;
					}
					if (plr.Value.DeadTimer == 0)
					{
						Packet pkt = new Packet(PacketType.SERVER_GAME_DEATHTIMER_END);
						if (DemonCount >= (int)((double)(Players.Count - 1) / 2.0))
						{
							pkt.Write(0);
						}
						else
						{
							DemonCount++;
							plr.Value.RevivalTimes = 2;
							Logger.Log(plr.Value.Nickname + " was demonized!", ConsoleColor.Red, ID);
							pkt.Write(1);
						}
						FindSession(plr.Key).SendAsync(pkt.ToArray());
						plr.Value.DeadTimer = -1;
					}
					else
					{
						Packet pk = new Packet(PacketType.SERVER_GAME_DEATHTIMER_TICK);
						pk.Write(plr.Value.ID);
						pk.Write((byte)plr.Value.DeadTimer);
						MulticastAsync(pk);
					}
				}
			}
		}
	}

	private void Countdown_Count(int count)
	{
		if (State == State.LOBBY)
		{
			if (count != 0)
			{
				Packet packet = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
				packet.Write((byte)1);
				packet.Write((byte)count);
				MulticastAsync(packet);
			}
		}
		else if (!GameEnded)
		{
			GameTick();
			Packet packet2 = new Packet(PacketType.SERVER_GAME_TIME_SYNC);
			packet2.Write((ushort)(count * 60));
			MulticastAsync(packet2);
		}
	}

	private void Countdown_Reached()
	{
		if (State == State.LOBBY)
		{
			Packet packet = new Packet(PacketType.SERVER_LOBBY_COUNTDOWN);
			packet.Write((byte)1);
			packet.Write((byte)1);
			MulticastAsync(packet);
			StartVote();
		}
		else if (!GameEnded)
		{
			Packet packet3 = new Packet(PacketType.SERVER_GAME_TIME_OVER);
			MulticastAsync(packet3);
			EndGame();
		}
		else
		{
			Packet packet2 = new Packet(PacketType.SERVER_GAME_BACK_TO_LOBBY);
			MulticastAsync(packet2);
			StartLobby();
		}
	}

	public void MulticastAsync(Packet packet, Guid? ignore = null)
	{
		foreach (TcpSession val in Sessions.Values)
		{
			if (!(val.Id == ignore))
			{
				val.SendAsync(packet.ToArray(), 0L, packet.Length);
			}
		}
	}

	public void MulticastAsync(byte[] arr, Guid? ignore = null)
	{
		foreach (TcpSession val in Sessions.Values)
		{
			if (!(val.Id == ignore))
			{
				val.SendAsync(arr);
			}
		}
	}

	public void MulticastSync(Packet packet, Guid? ignore = null)
	{
		foreach (TcpSession val in Sessions.Values)
		{
			if (!(val.Id == ignore))
			{
				val.Send(packet.ToArray(), 0L, packet.Length);
			}
		}
	}

	public void MulticastSync(byte[] arr, Guid? ignore = null)
	{
		foreach (TcpSession val in Sessions.Values)
		{
			if (!(val.Id == ignore))
			{
				val.Send(arr);
			}
		}
	}

	protected override TcpSession CreateSession()
	{
		return new ServerSession(this);
	}

	protected override void OnStarted()
	{
		Logger.Log($"Server started on port {base.Port}.", ConsoleColor.Green);
		Logger.SendDiscord($"Server started on port {base.Port}.", ID, $"Version {206}");
	}

	protected override void OnError(SocketError error)
	{
		Logger.Log($"Server caught an error: {error}", ConsoleColor.Red, ID);
	}
}
