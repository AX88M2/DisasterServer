using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class KindAndFair : Map
{
	public override void Init(Server server)
	{
		for (int i = 0; i < 11; i++)
		{
			Spawn(server, new KAFSpeedBooster
			{
				ID = (byte)i
			});
		}
		SetTime(server, 180);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 82)
		{
			byte nid = reader.ReadByte();
			bool isProj = reader.ReadBoolean();
			KAFSpeedBooster[] list = FindOfType<KAFSpeedBooster>();
			if (list == null)
			{
				return;
			}
			if (nid < list.Length)
			{
				KAFSpeedBooster act = list[nid];
				lock (server.Peers)
				{
					act.Activate(server, server.Peers[session.ID].ID, isProj);
				}
			}
		}
		base.PeerTCPMessage(server, session, reader);
	}

	protected override int GetRingSpawnCount()
	{
		return 31;
	}
}
