using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Threading;

namespace ExeNet;

public class UdpServer : IDisposable
{
	private UdpClient _client;

	private Thread? _readThread;

	public bool IsRunning { get; private set; }

	public int Port { get; private set; }

	public UdpServer(int port)
	{
		Port = port;
	}

	public void Dispose()
	{
		Stop();
		GC.SuppressFinalize(this);
	}

	public bool Start()
	{
		try
		{
			_client = new UdpClient(Port);
			_client.Client.ReceiveBufferSize = 128;
			_client.Client.SendBufferSize = 128;
			_client.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, optionValue: true);
		}
		catch (Exception e)
		{
			OnError(null, e.Message);
			return false;
		}
		if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
		{
			byte[] inValue = new byte[1];
			byte[] outValue = new byte[1];
			_client.Client.IOControl(-1744830452, inValue, outValue);
		}
		IsRunning = true;
		_readThread = new Thread(Run);
		_readThread.Name = "UdpServer Worker";
		_readThread.Start();
		OnReady();
		return true;
	}

	public void Send(IPEndPoint endpoint, ref byte[] data)
	{
		Send(endpoint, ref data, data.Length);
	}

	public void Send(IPEndPoint endpoint, ref byte[] data, int length)
	{
		_client.Send(data, length, endpoint);
	}

	public void Stop()
	{
		IsRunning = false;
		_readThread.Join();
	}

	private void Run()
	{
		IPEndPoint endpoint = new IPEndPoint(IPAddress.Any, Port);
		while (IsRunning)
		{
			byte[] data = _client.Receive(ref endpoint);
			if (endpoint != null)
			{
				try
				{
					OnData(endpoint, ref data);
				}
				catch (SocketException ex2)
				{
					OnSocketError(endpoint, ex2.SocketErrorCode);
				}
				catch (Exception ex)
				{
					OnError(endpoint, ex.Message);
				}
			}
		}
		_client.Close();
	}

	protected virtual void OnReady()
	{
	}

	protected virtual void OnSocketError(IPEndPoint endpoint, SocketError error)
	{
	}

	protected virtual void OnError(IPEndPoint? endpoint, string message)
	{
	}

	protected virtual void OnData(IPEndPoint sender, ref byte[] data)
	{
	}
}
