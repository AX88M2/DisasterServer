using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class GreenHill : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 180);
		Spawn<GHZThunder>(server);
		base.Init(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 26;
	}
}
