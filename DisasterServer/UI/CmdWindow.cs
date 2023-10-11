using System.Text.Json.Nodes;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.UI;

public class CmdWindow : Window
{
	public override void Log(string message)
	{
		Console.WriteLine(message);
	}

	public override bool Run()
	{
#if _WINDOWS
                UIWrapper.AllocConsole();
#endif
		Terminal.Log("===================");
		Terminal.Log("TD2DR Server");
		Terminal.Log($"BUILD v{Program.GAME_VER}");
		Terminal.Log("Server edited by MilesGlitch");
		Terminal.Log("(c) Team Exe Empire 2023");
		Terminal.Log("===================");
		Terminal.Log("Enter localhost or 127.0.0.1 on your PC to join the server.\n");
		string file = Options.Get<string>("mapset_file");
		if (!string.IsNullOrEmpty(file))
		{
			try
			{
				foreach (JsonNode node in JsonNode.Parse(File.ReadAllText(file)).Root.AsArray())
				{
					if (node != null)
					{
						MapVote.Excluded.Add((int)(JsonNode)node.AsValue());
					}
				}
			}
			catch (InvalidOperationException)
			{
				Terminal.Log("Failed to load mapset_file (Invalid format?)!");
			}
			catch
			{
				Terminal.Log("Failed to load mapset_file!");
			}
		}
		for (int i = 0; i < Options.Get<int>("server_count"); i++)
		{
			Server server = new Server(i);
			server.StartAsync();
			Program.Servers.Add(server);
		}
		while (true)
		{
			Thread.Sleep(1000);
		}
	}
}
