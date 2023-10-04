using System.Net;
using System.Text;
using DisasterServer.Data;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer;

public static class Ext
{
	public const int FRAMESPSEC = 60;

	private static Random _rand = new Random();

	public static string ReadStringNull(this BinaryReader reader)
	{
		List<byte> bytes = new List<byte>();
		byte c;
		while (reader.BaseStream.Position < reader.BaseStream.Length && (c = reader.ReadByte()) != 0)
		{
			bytes.Add(c);
		}
		return Encoding.UTF8.GetString(bytes.ToArray());
	}

	public static T? CreateOfType<T>()
	{
		return (T)Activator.CreateInstance(typeof(T));
	}

	public static T? CreateOfType<T>(Type value)
	{
		return (T)Activator.CreateInstance(value);
	}

	public static double Dist(double x, double y, double x2, double y2)
	{
		return Math.Sqrt(Math.Pow(x2 - x, 2.0) + Math.Pow(y2 - y, 2.0));
	}

	public static string ValidateNick(string nick)
	{
		string nick2 = nick;
		char[] array = new char[9] { '\\', '/', '@', '|', '№', '`', '~', '&', ' ' };
		foreach (char ch in array)
		{
			nick2 = nick2.Replace(ch.ToString(), "");
		}
		if (nick2.Length <= 0 || string.IsNullOrEmpty(nick2) || string.IsNullOrWhiteSpace(nick2))
		{
			return $"/player~ \\{_rand.Next(9999)}";
		}
		return nick;
	}

	public static void HandleIdentity(Server server, TcpSession session, BinaryReader reader)
	{
		ushort ver = reader.ReadUInt16();
		string name = reader.ReadStringNull();
		byte icon = reader.ReadByte();
		sbyte pet = reader.ReadSByte();
		OSType os = (OSType)reader.ReadByte();
		string udid = reader.ReadStringNull();
		if (ver != Program.BUILD_VER)
		{
			server.DisconnectWithReason(session, $"Wrong game version ({Program.BUILD_VER} required, but got {ver})");
		}
		else
		{
			if (server.Peers.Count >= 8)
			{
				return;
			}
			lock (server.Peers)
			{
				if (server.Peers.ContainsKey(session.ID))
				{
					server.Peers[session.ID].Pending = false;
					server.Peers[session.ID].Waiting = server.State.AsState() != Session.State.LOBBY;
					server.Peers[session.ID].Nickname = ValidateNick(name);
					server.Peers[session.ID].Icon = icon;
					server.Peers[session.ID].Pet = pet;
					server.Peers[session.ID].Unique = udid;
					if (server.State.AsState() == Session.State.LOBBY)
					{
						Terminal.LogDiscord($"{name} (ID {server.Peers[session.ID].ID}) joined.");
						TcpPacket packet = new TcpPacket(PacketType.SERVER_PLAYER_INFO);
						packet.Write(server.Peers[session.ID].ID);
						packet.Write(name);
						packet.Write(icon);
						packet.Write(pet);
						server.TCPMulticast(packet, session.ID);
					}
					Program.Window.AddPlayer(server.Peers[session.ID]);
				}
			}
			Program.Stat?.MulticastInformation();
			Terminal.LogDebug($"Indentity recived from {(IPEndPoint)session.RemoteEndPoint}:");
			Terminal.LogDebug($"  OS: {os}");
			Terminal.LogDebug("  UNIQUE: " + udid);
			if (KickList.Check(udid))
			{
				server.DisconnectWithReason(session, "Kick by host.");
				return;
			}
			if (BanList.Check(udid))
			{
				server.DisconnectWithReason(session, "Banned by host.");
				return;
			}
			TcpPacket pak = new TcpPacket(PacketType.SERVER_IDENTITY_RESPONSE);
			pak.Write(server.State.AsState() == Session.State.LOBBY);
			pak.Write(session.ID);
			server.TCPSend(session, pak);
		}
	}
}
