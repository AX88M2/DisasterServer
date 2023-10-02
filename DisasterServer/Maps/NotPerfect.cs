using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class NotPerfect : Map
{
	public override void Init(Server server)
	{
		Spawn<NotPerfectController>(server);
		SetTime(server, 155);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 59;
	}

	protected override float GetRingTime()
	{
		return 3f;
	}
}
