using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class DarkTower : Map
{
	public override void Init(Server server)
	{
		Spawn<DTTailsDoll>(server);
		Spawn<DTBall>(server);
		Spawn(server, new DTAss(1744, 224));
		Spawn(server, new DTAss(1840, 224));
		Spawn(server, new DTAss(1936, 224));
		Spawn(server, new DTAss(2032, 224));
		Spawn(server, new DTAss(2128, 224));
		Spawn(server, new DTAss(1824, 784));
		Spawn(server, new DTAss(1920, 784));
		Spawn(server, new DTAss(2016, 784));
		Spawn(server, new DTAss(2112, 784));
		Spawn(server, new DTAss(2208, 784));
		Spawn(server, new DTAss(2464, 1384));
		Spawn(server, new DTAss(2592, 1384));
		Spawn(server, new DTAss(3032, 64));
		Spawn(server, new DTAss(3088, 64));
		SetTime(server, 205);
		base.Init(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 31;
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 91)
		{
			byte id = reader.ReadByte();
			FindOfType<DTAss>()?.FirstOrDefault((DTAss e) => e.ID == id)?.Dectivate(server);
		}
		base.PeerTCPMessage(server, session, reader);
	}
}
