using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class VolcanoValley : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 180);
		Spawn(server, new VVLava(0, 736f, 130f));
		Spawn(server, new VVLava(1, 1388f, 130f));
		Spawn(server, new VVLava(2, 1524f, 130f));
		Spawn(server, new VVLava(3, 1084f, 130f));
		for (byte i = 0; i < 14; i = (byte)(i + 1))
		{
			Spawn(server, new VVVase(i));
		}
		base.Init(server);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 88)
		{
			byte nid = reader.ReadByte();
			VVVase[] list = FindOfType<VVVase>();
			if (list == null)
			{
				return;
			}
			VVVase vase = list.Where((VVVase e) => e.ID == nid).FirstOrDefault();
			if (vase != null)
			{
				vase.DestroyerID = session.ID;
				Destroy(server, vase);
			}
		}
		base.PeerTCPMessage(server, session, reader);
	}

	protected override int GetRingSpawnCount()
	{
		return 27;
	}
}
