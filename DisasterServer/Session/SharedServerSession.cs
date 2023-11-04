using System.Net;
using System.Net.Sockets;
using DisasterServer.Data;
using ExeNet;

namespace DisasterServer.Session;

public class SharedServerSession : TcpSession
{
	private Server _server;

	private List<byte> _header = new List<byte>();

	private List<byte> _data = new List<byte>();

	private int _length = -1;

	private bool _start;

	private byte[] _headerData = new byte[5] { 104, 80, 75, 84, 0 };

	public SharedServerSession(Server server, TcpClient client)
		: base(server.SharedServer, client)
	{
		_server = server;
	}

	protected override void OnConnected()
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		lock (_server.Peers)
		{
			if (KickList.Check((base.RemoteEndPoint as IPEndPoint).Address.ToString()))
			{
				_server.DisconnectWithReason(this, "Kicked by server.");
				return;
			}
			if (BanList.Check((base.RemoteEndPoint as IPEndPoint).Address.ToString()))
			{
				_server.DisconnectWithReason(this, "You were banned from this server.");
				return;
			}
			
			if (_server.Peers.Count >= Program.MAX_PLAYERS)
			{
				_server.DisconnectWithReason(this, "Server is full. (7/7)");
				return;
			}

			if (Options.Get<bool>("whitelist_enable"))
			{
				if (!Whitelist.Check((base.RemoteEndPoint as IPEndPoint).Address.ToString()))
				{
					_server.DisconnectWithReason(this, "Nope.");
					return;
				}
			}
			Peer peer = new Peer
			{
				EndPoint = base.RemoteEndPoint,
				Player = new Player(),
				ID = ID,
				Pending = true,
				Waiting = (_server.State.AsState() != State.LOBBY)
			};
			_server.Peers.Add(ID, peer);
			if (!peer.Waiting)
			{
				_server.State.PeerJoined(_server, this, peer);
			}
			Terminal.Log($"{base.RemoteEndPoint} (ID {ID}) connected.");
		}
		base.OnConnected();
	}

	protected override void OnDisconnected()
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		lock (_server.Peers)
		{
			Terminal.LogDebug("Disconnect Stage 1");
			_server.Peers.Remove(ID, out Peer peer);
			if (peer == null)
			{
				return;
			}
			Terminal.LogDebug("Disconnect Stage 2");
			Program.Window.RemovePlayer(peer);
			if (peer.Waiting)
			{
				Terminal.Log($"{peer?.EndPoint} (waiting) (ID {peer?.ID}) disconnected.");
				return;
			}
			Terminal.LogDebug("Disconnect Stage 3");
			TcpPacket packet = new TcpPacket(PacketType.SERVER_PLAYER_LEFT, peer.ID);
			_server.TCPMulticast(packet, ID);
			Terminal.LogDebug("Disconnect Stage 4");
			_server.State.PeerLeft(_server, this, peer);
			Terminal.Log($"{peer?.EndPoint} (ID {peer?.ID}) disconnected.");
		}
		Program.Stat?.MulticastInformation();
		base.OnDisconnected();
	}

	protected override void OnData(byte[] buffer, int length)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		ProcessBytes(buffer, length);
		base.OnData(buffer, length);
	}

	protected override void Timeouted()
	{
		_server.DisconnectWithReason(this, "Connection timeout");
		base.Timeouted();
	}

	private void ProcessBytes(byte[] buffer, int length)
	{
		using MemoryStream memStream = new MemoryStream(buffer, 0, length);
		while (memStream.Position < memStream.Length)
		{
			byte bt = (byte)memStream.ReadByte();
			if (_start)
			{
				_start = false;
				_length = bt;
				_data.Clear();
				Terminal.LogDebug($"Packet start {_length}");
			}
			else
			{
				_data.Add(bt);
				if (_data.Count >= _length && _length != -1)
				{
					byte[] data = _data.ToArray();
					using MemoryStream stream = new MemoryStream(data);
					using BinaryReader reader = new BinaryReader(stream);
					Terminal.LogDebug("Packet recv " + BitConverter.ToString(data));
					try
					{
						if (data.Length > 256)
						{
							Terminal.LogDiscord("TCP overload (data.Length > 256)");
							_server.DisconnectWithReason(this, "Packet overload > 256");
						}
						else
						{
							_server.State.PeerTCPMessage(_server, this, reader);
						}
					}
					catch (Exception e)
					{
						OnError(e.Message);
					}
					_length = -1;
					_data.Clear();
				}
			}
			_header.Add(bt);
			if (_header.Count >= 6)
			{
				_header.RemoveAt(0);
			}
			if (_header.SequenceEqual(_headerData))
			{
				_start = true;
			}
		}
		if (_data.Count < _length && _length != -1)
		{
			Terminal.LogDebug("Packet split, waiting for part to arrive.");
		}
	}

	protected override void OnSocketError(SocketError error)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord($"Caught SocketError: {error}");
		_server.State.TCPSocketError(this, error);
		base.OnSocketError(error);
	}

	protected override void OnError(string message)
	{
		Thread.CurrentThread.Name = $"Server {_server.UID}";
		Terminal.LogDiscord("Caught Error: " + message);
		base.OnError(message);
	}
}
