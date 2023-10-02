using System.Net;
using System.Net.Sockets;
using DisasterServer.Data;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.State;

public abstract class State
{
	public abstract void PeerJoined(Server server, TcpSession session, Peer peer);

	public abstract void PeerLeft(Server server, TcpSession session, Peer peer);

	public abstract void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader);

	public abstract void PeerUDPMessage(Server server, IPEndPoint IPEndPoint, ref byte[] data);

	public virtual void UDPSocketError(IPEndPoint endpoint, SocketError error)
	{
	}

	public virtual void TCPSocketError(TcpSession session, SocketError error)
	{
	}

	public abstract void Init(Server server);

	public abstract void Tick(Server server);

	public abstract DisasterServer.Session.State AsState();
}
