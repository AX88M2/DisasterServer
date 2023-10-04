using System.Net;
using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.UI;

public class MainWindow : Window
{
	public override void AddPlayer(Peer peer)
	{
		UIWrapper.PlayerData data = default(UIWrapper.PlayerData);
		data.state = 0;
		data.pid = peer.ID;
		data.name = peer.Nickname;
		data.character = ((peer.Player.Character == Character.Exe) ? ((UIWrapper.PlayerCharacter)(6 + peer.Player.ExeCharacter)) : ((UIWrapper.PlayerCharacter)peer.Player.Character));
		UIWrapper.gui_player_state(data);
	}

	public override void Log(string message)
	{
		UIWrapper.gui_log(message);
	}

	public override void RemovePlayer(Peer peer)
	{
		UIWrapper.PlayerData data = default(UIWrapper.PlayerData);
		data.state = 1;
		data.pid = peer.ID;
		data.name = peer.Nickname;
		data.character = ((peer.Player.Character == Character.Exe) ? ((UIWrapper.PlayerCharacter)(6 + peer.Player.ExeCharacter)) : ((UIWrapper.PlayerCharacter)peer.Player.Character));
		UIWrapper.gui_player_state(data);
	}

	public override bool Run()
	{
		try
		{
			if (!UIWrapper.gui_run(_Ready))
			{
				return false;
			}
			bool running = true;
			while (running)
			{
				UIWrapper.PollData data;
				while (UIWrapper.gui_poll_events(out data))
				{
					switch (data.type)
					{
					case UIWrapper.PollType.POLL_QUIT:
						running = false;
						break;
					case UIWrapper.PollType.POLL_CLEAR_EXCLUDES:
						MapVote.Excluded.Clear();
						MapVote.Excluded.Add(18);
						break;
					case UIWrapper.PollType.POLL_ADD_EXCLUDE:
						MapVote.Excluded.Add(data.value1);
						break;
					case UIWrapper.PollType.POLL_KICK:
						foreach (Server server in Program.Servers)
						{
							SharedServerSession session = server.GetSession(data.value1);
							if (session != null)
							{
								KickList.Add((session.RemoteEndPoint as IPEndPoint).Address.ToString());
								server.DisconnectWithReason(session, "Kicked by server.");
							}
						}
						break;
					case UIWrapper.PollType.POLL_BAN:
						foreach (Server server2 in Program.Servers)
						{
							SharedServerSession session2 = server2.GetSession(data.value1);
							if (session2 != null)
							{
								if (BanList.Ban(data.value1, out string name, out string unique))
								{
									UIWrapper.gui_add_ban(name, unique);
								}
								server2.DisconnectWithReason(session2, "Banned by server.");
							}
						}
						break;
					case UIWrapper.PollType.POLL_UNBAN:
						BanList.Unban(data.value2);
						break;
					case UIWrapper.PollType.POLL_BACKTOLOBBY:
						foreach (Server server3 in Program.Servers)
						{
							if (server3.State.AsState() != 0 && server3.State.AsState() != DisasterServer.Session.State.VOTE)
							{
								server3.SetState<Lobby>();
							}
						}
						break;
					case UIWrapper.PollType.POLL_EXEWIN:
						foreach (Server server4 in Program.Servers)
						{
							if (server4.State.AsState() == DisasterServer.Session.State.GAME)
							{
								(server4.State as Game).EndGame(server4, 0);
							}
						}
						break;
					case UIWrapper.PollType.POLL_SURVWIN:
						foreach (Server server5 in Program.Servers)
						{
							if (server5.State.AsState() == DisasterServer.Session.State.GAME)
							{
								(server5.State as Game).EndGame(server5, 1);
							}
						}
						break;
					case UIWrapper.PollType.POLL_PRACTICE:
						foreach (Server server6 in Program.Servers)
						{
							lock (server6.Peers)
							{
								if (server6.Peers.Count <= 0)
								{
									continue;
								}
							}
							if (server6.State.AsState() == DisasterServer.Session.State.LOBBY)
							{
								server6.SetState(new CharacterSelect(new FartZone()));
							}
						}
						break;
					}
				}
				Thread.Sleep(1);
			}
			Environment.Exit(0);
			return true;
		}
		catch (Exception value)
		{
			Console.WriteLine(value);
			return false;
		}
	}

	private void _Ready()
	{
		try
		{
			Terminal.Log("===================");
			Terminal.Log("TD2DR Server");
			Terminal.Log($"BUILD v{Program.BUILD_VER}");
			Terminal.Log("(c) Team Exe Empire 2023");
			Terminal.Log("===================");
			Terminal.Log("Enter localhost or 127.0.0.1 on your PC to join the server.\n");
			for (int i = 0; i < Options.Get<int>("server_count"); i++)
			{
				Server server = new Server(i);
				server.StartAsync();
				Program.Servers.Add(server);
			}
			foreach (KeyValuePair<string, Dictionary<string, string>> it in BanList.GetBanned())
			{
				UIWrapper.gui_add_ban(it.Value["name"], it.Key);
			}
		}
		catch
		{
		}
	}
}
