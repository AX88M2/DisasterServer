using System.Net;
using DisasterServer.Data;
using DisasterServer.State;
using ExeNet;

namespace DisasterServer.Session;

public class Server
{
	private const int TCP_PORT = 7606;

	private const int UDP_PORT = 8606;

	public int UID = -1;

	public bool IsRunning;

	public DisasterServer.State.State State = new Lobby();

	public Dictionary<ushort, Peer> Peers = new Dictionary<ushort, Peer>();

	public int LastMap = -1;

	public MulticastServer MulticastServer;

	public SharedServer SharedServer;

	private Thread? _thread;

	private int _hbTimer;

	public Server(int uid)
	{
		MulticastServer = new MulticastServer(this, 8606 + uid);
		SharedServer = new SharedServer(this, 7606 + uid);
		UID = uid + 1;
	}

	public void StartAsync()
	{
		if (!SharedServer.Start())
		{
			throw new Exception("Failed to start SharedServer (TCP)");
		}
		if (!MulticastServer.Start())
		{
			throw new Exception("Failed to start MulticastServer (UCP)");
		}
		IsRunning = true;
		_thread = new Thread((ThreadStart)delegate
		{
			while (IsRunning)
			{
				DoHeartbeat();
				Tick();
				Thread.Sleep(15);
			}
		});
		_thread.Priority = ThreadPriority.AboveNormal;
		_thread.Name = $"Server {UID}";
		_thread.Start();
	}

	public void Tick()
	{
		State.Tick(this);
	}

	public SharedServerSession? GetSession(ushort id)
	{
		return (SharedServerSession)SharedServer.GetSession(id);
	}

	public void TCPSend(TcpSession? session, TcpPacket packet)
	{
		if (session == null)
		{
			return;
		}
		try
		{
			byte[] arr = packet.ToArray();
			session.Send(arr, packet.Length);
		}
		catch (Exception e)
		{
			Terminal.LogDiscord($"TCPSend() Exception: {e}");
		}
	}

	public void TCPMulticast(TcpPacket packet, ushort? except = null)
	{
		try
		{
			byte[] arr = packet.ToArray();
			lock (Peers)
			{
				foreach (KeyValuePair<ushort, Peer> peer in Peers)
				{
					if (peer.Key != except)
					{
						GetSession(peer.Value.ID)?.Send(arr, packet.Length);
					}
				}
			}
		}
		catch (Exception e)
		{
			Terminal.LogDiscord($"TCPMulticast() Exception: {e}");
		}
	}

	public void UDPSend(IPEndPoint IPEndPoint, UdpPacket packet)
	{
		try
		{
			byte[] arr = packet.ToArray();
			MulticastServer.Send(IPEndPoint, ref arr, packet.Length);
		}
		catch (InvalidOperationException)
		{
		}
		catch (Exception e)
		{
			Terminal.LogDiscord($"UDPSend() Exception: {e}");
		}
	}

	public void UDPMulticast(ref List<IPEndPoint> IPEndPoints, UdpPacket packet, IPEndPoint? except = null)
	{
		try
		{
			byte[] arr = packet.ToArray();
			lock (IPEndPoints)
			{
				foreach (IPEndPoint IPEndPoint in IPEndPoints)
				{
					if (IPEndPoint != except)
					{
						MulticastServer.Send(IPEndPoint, ref arr, packet.Length);
					}
				}
			}
		}
		catch (InvalidOperationException)
		{
		}
		catch (Exception e)
		{
			Terminal.LogDiscord($"UDPMulticast() Exception: {e}");
		}
	}

	public void UDPMulticast(ref Dictionary<ushort, IPEndPoint> IPEndPoints, UdpPacket packet, IPEndPoint? except = null)
	{
		try
		{
			byte[] arr = packet.ToArray();
			lock (IPEndPoints)
			{
				foreach (KeyValuePair<ushort, IPEndPoint> IPEndPoint in IPEndPoints)
				{
					if (IPEndPoint.Value != except)
					{
						MulticastServer.Send(IPEndPoint.Value, ref arr, packet.Length);
					}
				}
			}
		}
		catch (InvalidOperationException)
		{
		}
		catch (Exception e)
		{
			Terminal.LogDiscord($"UDPMulticast() Exception: {e}");
		}
	}

	public void Passtrough(BinaryReader reader, TcpSession sender)
	{
		Terminal.LogDebug("Passtrough()");
		long pos = reader.BaseStream.Position;
		reader.BaseStream.Seek(0L, SeekOrigin.Begin);
		reader.ReadByte();
		TcpPacket pk = new TcpPacket((PacketType)reader.ReadByte());
		while (reader.BaseStream.Position < reader.BaseStream.Length)
		{
			pk.Write(reader.ReadByte());
		}
		TCPMulticast(pk, sender.ID);
		reader.BaseStream.Seek(pos, SeekOrigin.Begin);
		Terminal.LogDebug("Passtrough end()");
	}

	public void DisconnectWithReason(TcpSession? session, string reason)
	{
		TcpSession session2 = session;
		string reason2 = reason;
		Task.Run(delegate
		{
			if (session2 == null || !session2.IsRunning)
			{
				return;
			}
			Terminal.LogDebug($"Disconnecting cuz following balls ({session2.ID}): {reason2}");
			Thread.CurrentThread.Name = $"Server {UID}";
			try
			{
				_ = session2.RemoteEndPoint;
				ushort iD = session2.ID;
				lock (Peers)
				{
					if (!Peers.ContainsKey(iD))
					{
						Terminal.LogDiscord($"(ID {iD}) disconnect: {reason2}");
					}
					else
					{
						Peer peer = Peers[iD];
						Terminal.LogDiscord($"{peer.Nickname} (ID {peer.ID}) disconnect: {reason2}");
					}
				}
				TcpPacket tcpPacket = new TcpPacket(PacketType.SERVER_PLAYER_FORCE_DISCONNECT);
				tcpPacket.Write(reason2);
				TCPSend(session2, tcpPacket);
				session2.Disconnect();
			}
			catch (Exception value)
			{
				Terminal.LogDiscord($"Disconnect failed: {value}");
			}
		});
	}

	public void SetState<T>() where T : DisasterServer.State.State
	{
		object obj = Activator.CreateInstance(typeof(T));
		if (obj != null)
		{
			State = (DisasterServer.State.State)obj;
			State.Init(this);
			Terminal.LogDiscord($"Server state is {State} now");
		}
	}

	public void SetState<T>(T value) where T : DisasterServer.State.State
	{
		State = value;
		State.Init(this);
		Terminal.LogDiscord($"Server state is {State} now");
	}

	private void DoHeartbeat()
	{
		lock (Peers)
		{
			if (Peers.Count <= 0)
			{
				return;
			}
		}
		if (_hbTimer++ >= 120)
		{
			TcpPacket pk = new TcpPacket(PacketType.SERVER_HEARTBEAT);
			TCPMulticast(pk);
			Terminal.LogDebug("Server heartbeated.");
			_hbTimer = 0;
		}
	}
}
