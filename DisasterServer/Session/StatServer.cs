using System.Net.Sockets;
using ExeNet;

namespace DisasterServer.Session;

public class StatServer : TcpServer
{
	public StatServer()
		: base(System.Net.IPAddress.Any, 12084)
	{
	}

	public void MulticastInformation()
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
		lock (Sessions)
		{
			foreach (TcpSession session in Sessions)
			{
				session.Send(arr);
			}
		}
	}

	protected override TcpSession CreateSession(TcpClient client)
	{
		return new StatServerSession(this, client);
	}
}
