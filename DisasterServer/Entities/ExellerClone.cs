using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class ExellerClone : Entity
{
	public byte ID;

	public ushort PID;

	public sbyte Dir;

	public ExellerClone(ushort pid, byte id, int x, int y, sbyte dir)
	{
		Dir = dir;
		PID = pid;
		ID = id;
		X = x;
		Y = y;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_EXELLERCLONE_STATE, (byte)1, ID);
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_EXELLERCLONE_STATE, (byte)0, ID, PID, (ushort)X, (ushort)Y, Dir);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		return null;
	}
}
