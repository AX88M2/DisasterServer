using System.Text;
using System.Text.Json;

namespace DisasterServer;

public class Terminal
{
	private static StringBuilder _builder;

	private static int _lines;

	private static string _fname;

	static Terminal()
	{
		_builder = new StringBuilder();
		_lines = 0;
		try
		{
			if (!Directory.Exists("Logs"))
			{
				Directory.CreateDirectory("Logs");
			}
			_fname = $"Logs/{DateTime.Now:yyyyMMddTHHmmss}.log";
			AppDomain currentDomain = AppDomain.CurrentDomain;
			currentDomain.UnhandledException += CurrentDomain_UnhandledException;
			currentDomain.ProcessExit += CurrentDomain_ProcessExit;
		}
		catch
		{
			Console.WriteLine("Logging to file is disabled due to an error.");
		}
	}

	private static void CurrentDomain_ProcessExit(object? sender, EventArgs e)
	{
		try
		{
			File.AppendAllText(_fname, _builder.ToString());
		}
		catch
		{
		}
	}

	private static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
	{
		try
		{
			File.AppendAllText(_fname, _builder.ToString());
		}
		catch
		{
		}
	}

	public static void Log(string text)
	{
		string time = DateTime.Now.ToLongTimeString();
		string msg = $"[{time} {Thread.CurrentThread.Name} INFO] {text}";
		Console.ForegroundColor = ConsoleColor.White;
		Program.Window.Log(msg);
		lock (_builder)
		{
			_builder.AppendLine(msg);
			if (_lines++ > 20)
			{
				File.AppendAllText(_fname, _builder.ToString());
				_builder.Clear();
			}
		}
	}

	public static void LogDebug(string text)
	{
		string time = DateTime.Now.ToLongTimeString();
		string msg = $"[{time} {Thread.CurrentThread.Name} DEBUG] {text}";
		lock (_builder)
		{
			_builder.AppendLine(msg);
			if (_lines++ > 20)
			{
				File.AppendAllText(_fname, _builder.ToString());
				_builder.Clear();
			}
		}
		if (Options.Get<bool>("debug_mode"))
		{
			Console.ForegroundColor = ConsoleColor.Yellow;
			Program.Window.Log(msg);
		}
	}

	public static void LogDiscord(string text)
	{
		string time = DateTime.Now.ToLongTimeString();
		string msg = $"[{time} {Thread.CurrentThread.Name} INFO] {text}";
		Console.ForegroundColor = ConsoleColor.White;
		Program.Window.Log(msg);
		lock (_builder)
		{
			_builder.AppendLine(msg);
			if (_lines++ > 20)
			{
				File.AppendAllText(_fname, _builder.ToString());
				_builder.Clear();
			}
		}
		if (!string.IsNullOrEmpty(Options.Get<string>("webhook_url")))
		{
			SendDiscord(text, Thread.CurrentThread.Name);
		}
	}

	public static void SendDiscord(string message, string? name, string title = "")
	{
		var @struct = new
		{
			username = "Thread (" + name + ")",
			embeds = new List<object>
			{
				new
				{
					color = "2829617",
					title = title,
					description = "``` " + message + " ```"
				}
			}
		};
		string json = JsonSerializer.Serialize(@struct);
		ThreadPool.QueueUserWorkItem(async delegate
		{
			try
			{
				await new HttpClient
				{
					Timeout = TimeSpan.FromSeconds(2.0)
				}.PostAsync(content: new StringContent(json, Encoding.UTF8, "application/json"), requestUri: Options.Get<string>("webhook_url"));
			}
			catch
			{
			}
		});
	}
}
