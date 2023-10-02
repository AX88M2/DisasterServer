using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class TailsProjectile : Entity
{
	public ushort OwnerID;

	public sbyte Direction;

	public bool IsExe;

	public byte Charge;

	public byte Damage;

	private int _timer = 300;

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		UdpPacket upacket = new UdpPacket(PacketType.SERVER_TPROJECTILE_STATE, (byte)1, (ushort)X, (ushort)Y, OwnerID, Direction, Damage, IsExe, Charge);
		server.UDPMulticast(ref game.IPEndPoints, upacket);
		return new TcpPacket(PacketType.SERVER_TPROJECTILE_STATE, (byte)0, (ushort)X, (ushort)Y, OwnerID, Direction, Damage, IsExe, Charge);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_TPROJECTILE_STATE, (byte)2);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		X += Direction * 12;
		if (X <= 0 || _timer-- <= 0)
		{
			map.Destroy(server, this);
			return null;
		}
		return new UdpPacket(PacketType.SERVER_TPROJECTILE_STATE, (byte)1, (ushort)X, (ushort)Y);
	}
}
