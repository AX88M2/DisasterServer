using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class NotPerfect : Map
{
	public override void Init(Server server)
	{
		Spawn<NotPerfectController>(server);
		if (Options.Get<bool>("random_mode"))
		{
			Random random = new Random();
			int addTimeRandom = random.Next(1, 128);
			SetTime(server, 155+addTimeRandom);
			Terminal.Log($"[NotPerfect] Time added: {addTimeRandom}");
		}
		else
		{
			SetTime(server, 155);
		}
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
