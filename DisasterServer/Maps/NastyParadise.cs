using DisasterServer.Entities;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.Maps;

public class NastyParadise : Map
{
	private int _timer;

	public override void Init(Server server)
	{
		for (byte m = 0; m < 10; m = (byte)(m + 1))
		{
			Spawn(server, new NAPIce(m));
		}
		NAPSnowball ball = Spawn(server, new NAPSnowball(0, 10, 1));
		for (byte l = 0; l < 4; l = (byte)(l + 1))
		{
			ball.SetWaypointMoveSpeed((byte)(5 + l), 0.05f + 0.05f * ((float)(int)l / 4f));
			ball.SetWaypointAnimSpeed((byte)(5 + l), 0.35f + 0.25f * ((float)(int)l / 4f));
		}
		ball = Spawn(server, new NAPSnowball(1, 8, -1));
		for (byte k = 0; k < 5; k = (byte)(k + 1))
		{
			ball.SetWaypointMoveSpeed((byte)(2 + k), 0.05f + 0.05f * ((float)(int)k / 5f));
			ball.SetWaypointAnimSpeed((byte)(2 + k), 0.35f + 0.25f * ((float)(int)k / 5f));
		}
		ball = Spawn(server, new NAPSnowball(2, 11, 1));
		for (byte j = 0; j < 5; j = (byte)(j + 1))
		{
			ball.SetWaypointMoveSpeed((byte)(5 + j), 0.05f + 0.05f * ((float)(int)j / 5f));
			ball.SetWaypointAnimSpeed((byte)(5 + j), 0.35f + 0.25f * ((float)(int)j / 5f));
		}
		ball = Spawn(server, new NAPSnowball(3, 9, 1));
		for (byte i = 0; i < 2; i = (byte)(i + 1))
		{
			ball.SetWaypointMoveSpeed((byte)(6 + i), 0.05f + 0.05f * ((float)(int)i / 2f));
			ball.SetWaypointAnimSpeed((byte)(6 + i), 0.35f + 0.25f * ((float)(int)i / 2f));
		}
		Spawn(server, new NAPSnowball(4, 5, -1));
		SetTime(server, 155);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		_timer++;
		if (_timer >= 1200)
		{
			_timer = 0;
			NAPSnowball[] array = FindOfType<NAPSnowball>();
			for (int i = 0; i < array.Length; i++)
			{
				array[i].Activate(server);
			}
		}
		base.Tick(server);
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		reader.ReadBoolean();
		if (reader.ReadByte() == 85)
		{
			byte id = reader.ReadByte();
			FindOfType<NAPIce>()?.Where((NAPIce e) => e.ID == id).FirstOrDefault()?.Activate(server);
		}
		base.PeerTCPMessage(server, session, reader);
	}

	protected override int GetRingSpawnCount()
	{
		return 26;
	}
}
