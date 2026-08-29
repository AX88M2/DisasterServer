using System;
using System.IO;
using System.Net.Sockets;
using NetCoreServer;
using Server.Game;

namespace Server;

internal class ServerSessionBase : TcpSession
{
	protected global::Server.Game.Server _server;

	public static readonly byte[] START_SIGNATURE = new byte[4] { 80, 111, 82, 110 };

	private byte[] _buffer = new byte[4];

	private byte _stage;

	private byte _index;

	public static readonly byte[] END_SIGNATURE = new byte[3] { 75, 105, 68 };

	public ServerSessionBase(global::Server.Game.Server server)
		: base(server)
	{
		_server = server;
	}

	protected override void OnConnected()
	{
		Logger.Log($"{base.Socket.RemoteEndPoint} joined. (GUID {base.Id})", ConsoleColor.Green);
		base.OnConnected();
	}

	protected override void OnDisconnecting()
	{
		lock (_server.Players)
		{
			if (_server.Players.ContainsKey(base.Id))
			{
				Logger.Log($"{_server.Players[base.Id].EndPoint} left. (GUID {base.Id})", ConsoleColor.Green);
			}
		}
		base.OnDisconnecting();
	}

	protected override void OnReceived(byte[] buffer, long offset, long size)
	{
		lock (_server.Players)
		{
			if (_server.Players.ContainsKey(base.Id))
			{
				_server.Players[base.Id].LastPacketTime = 0;
			}
		}
		using MemoryStream stream = new MemoryStream(buffer, (int)offset, (int)size);
		while (stream.Position < stream.Length)
		{
			switch (_stage)
			{
			case 0:
				if ((byte)stream.ReadByte() == START_SIGNATURE[_index])
				{
					_index++;
				}
				else if (_index > 0)
				{
					_index = 0;
				}
				if (_index >= START_SIGNATURE.Length)
				{
					_buffer = new byte[4];
					_index = 0;
					_stage = 1;
				}
				break;
			case 1:
			{
				byte sz = (byte)stream.ReadByte();
				_buffer = new byte[sz];
				_index = 0;
				_stage = 2;
				break;
			}
			case 2:
				_buffer[_index++] = (byte)stream.ReadByte();
				if (_index >= _buffer.Length)
				{
					_index = 0;
					_stage = 3;
				}
				break;
			case 3:
				if ((byte)stream.ReadByte() == END_SIGNATURE[_index])
				{
					_index++;
				}
				else if (_index > 0)
				{
					Logger.Log($"Broken packet recieved from {base.Id} (End signature is missing)!", ConsoleColor.Red);
					_stage = 0;
					_index = 0;
				}
				if (_index >= END_SIGNATURE.Length)
				{
					_index = 0;
					_stage = 4;
				}
				break;
			case 4:
				PacketReady(_buffer);
				_index = 0;
				_stage = 0;
				break;
			}
		}
		if (_stage == 3 && _index >= END_SIGNATURE.Length)
		{
			_index = 0;
			_stage = 4;
		}
		if (_stage == 4)
		{
			PacketReady(_buffer);
			_index = 0;
			_stage = 0;
		}
	}

	protected virtual void PacketReady(byte[] packet)
	{
	}

	protected override void OnError(SocketError error)
	{
		Logger.Log($"Error in session GUID ${base.Id}: {error}", ConsoleColor.Green);
	}

	public void Send(Packet packet)
	{
		Send(packet.ToArray(), 0L, packet.Length);
	}

	public void SendAsync(Packet packet)
	{
		SendAsync(packet.ToArray(), 0L, packet.Length);
	}
}
