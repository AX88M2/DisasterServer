using System.Net;

namespace DisasterServer.Data;

public class Peer
{
	public ushort ID;

	public string Nickname = "Pending...";

	public string Unique = "";

	public int ExeChance;

	public byte Icon;

	public sbyte Pet = -1;

	public bool Pending = true;

	public bool Waiting;

	public EndPoint EndPoint;

	public Player Player;
}
