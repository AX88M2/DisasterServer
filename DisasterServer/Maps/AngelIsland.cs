using DisasterServer.Session;

namespace DisasterServer.Maps;

internal class AngelIsland : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 180);
		base.Init(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 21;
	}
}
