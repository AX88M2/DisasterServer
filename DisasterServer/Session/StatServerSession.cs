using System.Net.Sockets;
using ExeNet;

namespace DisasterServer.Session;

public class StatServerSession : TcpSession
{
	public StatServerSession(TcpServer server, TcpClient client)
		: base(server, client)
	{
	}

	protected override void OnConnected()
	{
		byte[] arr = new byte[64];
		arr[0] = (byte)Program.Servers.Count;
		arr[1] = 7;
		int ind = 2;
		foreach (Server server in Program.Servers)
		{
			lock (server.Peers)
			{
				byte state = (byte)server.State.AsState();
				byte players = (byte)server.Peers.Count;
				arr[ind++] = state;
				arr[ind++] = players;
			}
		}
		Send(arr);
		base.OnConnected();
	}
}
