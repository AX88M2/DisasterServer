using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class LimpCity : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 155);
		Spawn(server, new LCEye
		{
			ID = 0
		});
		Spawn(server, new LCEye
		{
			ID = 1
		});
		Spawn<LCChainController>(server);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 81)
		{
			bool value = reader.ReadBoolean();
			byte nid = reader.ReadByte();
			byte target = reader.ReadByte();
			lock (Entities)
			{
				LCEye[] list = FindOfType<LCEye>();
				if (list != null && nid < list.Length)
				{
					LCEye eye = list[nid];
					if (value)
					{
						if (!eye.Used && eye.Charge >= 20)
						{
							eye.UseID = session.ID;
							eye.Target = target;
							eye.Used = true;
							eye.SendState(server);
						}
					}
					else
					{
						eye.Used = false;
						eye.SendState(server);
					}
				}
			}
		}
		base.PeerTCPMessage(server, session, reader);
	}

	protected override int GetRingSpawnCount()
	{
		return 23;
	}
}
