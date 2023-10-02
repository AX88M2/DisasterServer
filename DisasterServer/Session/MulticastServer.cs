using System.Net;
using System.Net.Sockets;
using ExeNet;

namespace DisasterServer.Session;

public class MulticastServer : UdpServer
{
	protected Server _server;

	public MulticastServer(Server server, int port)
		: base(port)
	{
		_server = server;
	}

	protected override void OnReady()
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord($"Server started on UDP port {base.Port}");
		base.OnReady();
	}

	protected override void OnSocketError(IPEndPoint endpoint, SocketError error)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord($"Caught SocketError: {error}");
		_server.State.UDPSocketError(endpoint, error);
		base.OnSocketError(endpoint, error);
	}

	protected override void OnError(IPEndPoint? endpoint, string message)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord("Caught Error: " + message);
		base.OnError(endpoint, message);
	}

	protected override void OnData(IPEndPoint sender, ref byte[] data)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		if (data.Length > 128)
		{
			Terminal.LogDiscord("UDP overload (data.Length > 128)");
			return;
		}
		_server.State.PeerUDPMessage(_server, sender, ref data);
		base.OnData(sender, ref data);
	}
}
