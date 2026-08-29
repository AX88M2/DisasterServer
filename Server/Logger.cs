using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Text;
using System.Text.Json;
using System.Threading;

namespace Server;

internal class Logger
{
	public static void Log(string text, ConsoleColor color = ConsoleColor.Gray, int serverId = -1)
	{
		Console.ForegroundColor = color;
		Console.WriteLine("[Server " + DateTime.Now.ToLongTimeString() + "] " + text);
		SendDiscord(text, serverId);
	}

	public static void SendDiscord(string message, int serverId, string title = "")
	{
		if (serverId != -1 && Program.WebHook != null)
		{
			var @struct = new
			{
				username = $"Server ({serverId})",
				embeds = new List<object>
				{
					new
					{
						title = title,
						description = "``` " + message + " ```"
					}
				}
			};
			string json = JsonSerializer.Serialize(@struct);
			ThreadPool.QueueUserWorkItem(async delegate
			{
				await new HttpClient().PostAsync(content: new StringContent(json, Encoding.UTF8, "application/json"), requestUri: Program.WebHook);
			});
		}
	}
}
