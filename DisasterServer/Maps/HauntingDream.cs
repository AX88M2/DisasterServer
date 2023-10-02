using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class HauntingDream : Map
{
	public override void Init(Server server)
	{
		Spawn<HDDoor>(server);
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
		if (reader.ReadByte() == 92)
		{
			FindOfType<HDDoor>()?.FirstOrDefault()?.Toggle(server);
		}
		base.PeerTCPMessage(server, session, reader);
	}
}
