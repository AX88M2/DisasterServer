using System.Net;
using System.Text.Json;
using DisasterServer.Session;

namespace DisasterServer;

public class Whitelist
{
    private static Ban _list;
    
    	static Whitelist()
    	{
    		_list = new Ban();
    		try
    		{
    			if (!File.Exists("Config/Whitelist.json"))
    			{
    				WriteDefault();
    			}
    			else
    			{
    				_list = JsonSerializer.Deserialize<Ban>(File.ReadAllText("Config/Whitelist.json"));
    			}
    		}
    		catch
    		{
    			Terminal.Log("Failed to load whitelist.");
    		}
    	}
    
    	public static void WriteDefault()
    	{
    		try
    		{
    			string ser = JsonSerializer.Serialize(_list);
    			if (!Directory.Exists("Config"))
    			{
    				Directory.CreateDirectory("Config");
    			}
    			File.WriteAllText("Config/Whitelist.json", ser);
    		}
    		catch
    		{
    			Terminal.Log("Failed to save whitelist.");
    		}
    	}
    
    	public static bool Add(ushort pid, out string nickname, out string unique)
    	{
    		unique = "";
    		nickname = "";
    		JsonSerializerOptions options = new JsonSerializerOptions();
    		options.WriteIndented = true;
    		foreach (Server server in Program.Servers)
    		{
    			lock (server.Peers)
    			{
    				if (!server.Peers.ContainsKey(pid))
    				{
    					continue;
    				}
    				unique = server.Peers[pid].Unique;
    				nickname = server.Peers[pid].Nickname;
    				string ip = (server.Peers[pid].EndPoint as IPEndPoint).Address.ToString();
    				Dictionary<string, string> @struct = new Dictionary<string, string>
    				{
    					{ "name", nickname },
    					{ "ip", ip }
    				};
    				_list.List[unique] = @struct;
    				string ball = JsonSerializer.Serialize(_list,options);
    				File.WriteAllText("Config/Writelist.json", ball);
    				return true;
    			}
    		}
    		return false;
    	}
    
    	public static void Remove(string unqie)
    	{
    		if (_list.List.ContainsKey(unqie))
    		{
    			_list.List.Remove(unqie);
    			File.WriteAllText("Config/Writelist.json", JsonSerializer.Serialize(_list));
    		}
    	}
    
    	public static bool Check(string unqie)
    	{
    		if (!_list.List.ContainsKey(unqie))
    		{
    			return false;
    		}
    		return true;
    	}
    
    	public static Dictionary<string, Dictionary<string, string>> GetPlayers()
    	{
    		return _list.List;
    	}
}