using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

internal class FartZone : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 256);
		Spawn<Fart>(server);
		Spawn<MovingSpikeController>(server);
		Spawn<DTBall>(server);
		for (int i = 0; i < 3; i++)
		{
			Spawn<BlackRing>(server);
		}
		base.Init(server);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 93)
		{
			sbyte spd = reader.ReadSByte();
			Fart[] list = FindOfType<Fart>();
			if (list == null)
			{
				return;
			}
			list[0].Push(spd);
		}
		base.PeerTCPMessage(server, session, reader);
	}

	protected override int GetRingSpawnCount()
	{
		return 15;
	}

	protected override float GetRingTime()
	{
		return 1f;
	}
}
