using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class LCEye : Entity
{
	public byte ID;

	public ushort UseID;

	public bool Used;

	public int Charge = 100;

	public byte Target;

	private int _cooldown;

	private int _timer;

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (_cooldown > 0)
		{
			_cooldown--;
			return null;
		}
		if (_timer++ >= 60)
		{
			_timer = 0;
			if (Used && Charge > 0)
			{
				Charge -= 20;
				if (Charge < 20)
				{
					_cooldown = 120;
					Used = false;
				}
				SendState(server);
			}
			else if (!Used && Charge < 100)
			{
				Charge += 10;
				SendState(server);
			}
		}
		return null;
	}

	public void SendState(Server server)
	{
		TcpPacket packet = new TcpPacket(PacketType.SERVER_LCEYE_STATE);
		packet.Write(ID);
		packet.Write(Used);
		packet.Write(UseID);
		packet.Write(Target);
		packet.Write((byte)Charge);
		server.TCPMulticast(packet);
	}
}
