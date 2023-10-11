using DisasterServer.Session;
using DisasterServer.UI;

namespace DisasterServer;

public class Program
{
	public const int GAME_VER = 100;
	public const String BUILD_VER = "v1.0.4";

	public const int MAX_PLAYERS = 7;

	public static List<Server> Servers { get; private set; } = new List<Server>();


	public static Window Window { get; private set; }

	public static StatServer? Stat { get; private set; }

	public static void Main(string[] args)
	{
		if (Options.Get<int>("server_count") <= 0)
		{
#if _WINDOWS
                UIWrapper.AllocConsole();
#endif
			Console.ForegroundColor = ConsoleColor.DarkRed;
			Console.WriteLine("ServerCount is set to 0 in config.");
			return;
		}
		if (Options.Get<bool>("enable_stat"))
		{
			Stat = new StatServer();
			Stat.Start();
		}
		Window = ((Options.Get<bool>("console_mode") || Options.Get<int>("server_count") > 1) ? ((Window)new CmdWindow()) : ((Window)new MainWindow()));
		if (!Window.Run())
		{
			Window = new CmdWindow();
			Window.Run();
		}
	}

	public static Server? FindServer(string id)
	{
		if (!int.TryParse(id, out var srvId))
		{
			return null;
		}
		srvId--;
		if (srvId < 0 || srvId >= Servers.Count)
		{
			return null;
		}
		return Servers[srvId];
	}
}
