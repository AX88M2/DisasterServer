using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class YouCantRun : Map
{
	public override void Init(Server server)
	{
		Random random = new Random();
		int addTimeRandom = random.Next(1, 128);
		Spawn<MovingSpikeController>(server);
		Spawn<YCRSmokeController>(server);
		SetTime(server, 180+addTimeRandom);
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	protected override int GetRingSpawnCount()
	{
		return 27;
	}
}
