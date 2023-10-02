using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class TortureCave : Map
{
	public override void Init(Server server)
	{
		Spawn<TCGom>(server);
		SetTime(server, 155);
		base.Init(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 27;
	}
}
