using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

internal class EggmanTracker : Entity
{
	public static byte TrackerIDs;

	public ushort ActivatorID;

	public byte ID;

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		ID = TrackerIDs++;
		return new TcpPacket(PacketType.SERVER_ETRACKER_STATE, (byte)0, ID, (ushort)X, (ushort)Y);
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return new TcpPacket(PacketType.SERVER_ETRACKER_STATE, (byte)1, ID, ActivatorID);
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		return null;
	}
}
