using System.Text.Json.Serialization;

namespace DisasterServer;

public class Ban
{
	[JsonPropertyName("list")]
	public Dictionary<string, Dictionary<string, string>> List { get; set; } = new Dictionary<string, Dictionary<string, string>>();

}
