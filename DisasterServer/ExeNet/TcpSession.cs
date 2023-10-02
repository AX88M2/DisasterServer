using System;
using System.Net;
using System.Net.Sockets;

namespace ExeNet;

public class TcpSession : IDisposable
{
	public ushort ID;

	private byte[] _readBuffer = Array.Empty<byte>();

	public int ReadBufferSize { get; private set; } = 96;


	public bool IsRunning { get; private set; }

	public EndPoint? RemoteEndPoint
	{
		get
		{
			try
			{
				if (Client == null)
				{
					return null;
				}
				if (Client.Client == null)
				{
					return null;
				}
				if (!Client.Connected)
				{
					return null;
				}
				return Client.Client.RemoteEndPoint;
			}
			catch
			{
				return null;
			}
		}
	}

	protected TcpClient Client { get; private set; }

	protected TcpServer Server { get; private set; }

	public TcpSession(TcpServer server, TcpClient client)
	{
		Server = server;
		Client = client;
		ID = server.RequestID();
		Client.SendTimeout = 3000;
		Client.ReceiveTimeout = 2000;
		Client.NoDelay = true;
	}

	public void Start()
	{
		_readBuffer = new byte[ReadBufferSize];
		IsRunning = true;
		OnConnected();
		if (IsRunning)
		{
			Client.Client.BeginReceive(_readBuffer, 0, _readBuffer.Length, SocketFlags.None, DoReceive, null);
		}
	}

	public void Send(byte[] data)
	{
		Send(data, data.Length);
	}

	public void Send(byte[] data, int length)
	{
		if (IsRunning && Client.Connected)
		{
			Client.Client.BeginSend(data, 0, length, SocketFlags.None, DoSend, null);
		}
	}

	public void Disconnect()
	{
		Console.WriteLine($"[TcpSession.cs] CleanUp call from Disconnect ({IsRunning})");
		CleanUp();
	}

	private void CleanUp()
	{
		try
		{
			Console.WriteLine("[TcpSession.cs] CleanUp()");
			if (!IsRunning)
			{
				return;
			}
			IsRunning = false;
			if (Client.Connected)
			{
				Client.Close();
			}
			Console.WriteLine("[TcpSession.cs] Client is closed");
			OnDisconnected();
			lock (Server.Sessions)
			{
				if (Server.Sessions.Contains(this))
				{
					Server.Sessions.Remove(this);
				}
			}
			Console.WriteLine("[TcpSession.cs] Disconnect called and removed from the list.");
		}
		catch (ObjectDisposedException)
		{
			Console.WriteLine("[TcpSession.cs] already disposed");
		}
	}

	private void DoSend(IAsyncResult result)
	{
		try
		{
			if (!IsRunning)
			{
				return;
			}
			SocketError code;
			int length = Client.Client.EndSend(result, out code);
			Console.WriteLine($"[TcpSession.cs] DoSend() SocketError Code: {code}");
			switch (code)
			{
			case SocketError.NetworkReset:
			case SocketError.ConnectionAborted:
			case SocketError.ConnectionReset:
			case SocketError.Shutdown:
			case SocketError.ConnectionRefused:
				CleanUp();
				break;
			default:
				if (length <= 0)
				{
					CleanUp();
				}
				else
				{
					OnSocketError(code);
				}
				break;
			case SocketError.Success:
				if (length <= 0)
				{
					CleanUp();
				}
				break;
			case SocketError.TimedOut:
				break;
			}
		}
		catch (Exception e)
		{
			OnError(e.Message);
		}
	}

	private void DoReceive(IAsyncResult result)
	{
		try
		{
			if (!IsRunning || Client == null)
			{
				return;
			}
			SocketError code;
			int length = Client.Client.EndReceive(result, out code);
			Console.WriteLine($"[TcpSession.cs] DoReceive() SocketError Code: {code}");
			switch (code)
			{
			case SocketError.NetworkReset:
			case SocketError.ConnectionAborted:
			case SocketError.ConnectionReset:
			case SocketError.Shutdown:
			case SocketError.ConnectionRefused:
				CleanUp();
				return;
			default:
				if (length <= 0)
				{
					CleanUp();
					break;
				}
				OnSocketError(code);
				CleanUp();
				return;
			case SocketError.Success:
				if (length <= 0)
				{
					CleanUp();
					return;
				}
				break;
			case SocketError.TimedOut:
				break;
			}
			OnData(_readBuffer, length);
		}
		catch (Exception e2)
		{
			OnError(e2.Message);
		}
		if (!IsRunning)
		{
			return;
		}
		try
		{
			Client.Client.BeginReceive(_readBuffer, 0, _readBuffer.Length, SocketFlags.None, DoReceive, null);
		}
		catch (Exception e)
		{
			OnError(e.Message);
		}
	}

	protected virtual void OnConnected()
	{
	}

	protected virtual void OnData(byte[] data, int length)
	{
	}

	protected virtual void OnSocketError(SocketError error)
	{
	}

	protected virtual void OnError(string message)
	{
	}

	protected virtual void Timeouted()
	{
	}

	protected virtual void OnDisconnected()
	{
	}

	public void Dispose()
	{
		Console.WriteLine($"[TcpSession.cs] CleanUp call from Dispose ({IsRunning})");
		CleanUp();
		Client.Dispose();
	}
}
