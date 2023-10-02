using System.Net.Sockets;
using ExeNet;

namespace DisasterServer.Session;

public class SharedServer : TcpServer
{
	protected Server _server;

	public SharedServer(Server server, int port)
		: base(System.Net.IPAddress.Any, port)
	{
		_server = server;
	}

	protected override void OnReady()
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord($"Server started on TCP port {base.Port}");
		base.OnReady();
	}

	protected override void OnSocketError(SocketError error)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord($"Caught SocketError: {error}");
		base.OnSocketError(error);
	}

	protected override void OnError(string message)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord("Caught Error: " + message);
		base.OnError(message);
	}

	protected override TcpSession CreateSession(TcpClient client)
	{
		return new SharedServerSession(_server, client);
	}
}
