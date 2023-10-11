using System.Text.Json;
using System.Text.Json.Nodes;

namespace DisasterServer;

public class Options
{
	private static JsonNode _doc;

	private static string _path;

	static Options()
	{
		_path = "Config/Options.json";
		try
		{
			if (Environment.GetCommandLineArgs().Length > 1)
			{
				_path = Environment.GetCommandLineArgs()[1];
			}
			if (!File.Exists(_path))
			{
				WriteDefault();
			}
			_doc = JsonNode.Parse(File.ReadAllText(_path));
		}
		catch
		{
			Terminal.Log("Failed to load config.");
		}
	}

	private static void WriteDefault()
	{
		try
		{
			string ser = JsonSerializer.Serialize(new
			{
				server_count = 1,
				tcp_port = 7606,
				udp_port = 8606,
				webhook_url = "",
				mapset_file = "",
				enable_stat = false,
				console_mode = true,
				antiAfk_system = true,
				debug_mode = false
			});
			_doc = JsonNode.Parse(ser);
			if (!Directory.Exists("Config"))
			{
				Directory.CreateDirectory("Config");
			}
			File.WriteAllText(_path, ser);
		}
		catch
		{
			Terminal.Log("Failed to save config.");
		}
	}

	public static void Set(string key, dynamic value)
	{
		_doc[key] = value;
		File.WriteAllText(_path, _doc.ToJsonString());
	}

	public static T? Get<T>(string key)
	{
		return _doc[key].AsValue().Deserialize<T>();
	}
}
