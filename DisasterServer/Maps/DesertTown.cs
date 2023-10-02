using DisasterServer.Session;

namespace DisasterServer.Maps;

public class DesertTown : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 180);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 25;
	}
}
