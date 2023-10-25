using DisasterServer.Data;
using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class Act9 : Map
{
	public override void Init(Server server)
	{
		Random random = new Random();
		int wallRandom = random.Next(1, 8);
		int addTimeRandom = random.Next(1,128);
		SetTime(server, 130+addTimeRandom);
		switch (wallRandom)
		{
			case 1:
				Spawn(server, new Act9Wall(0, 0, 1025)); // Поталог
				Spawn(server, new Act9Wall(1, 1663, 0)); // Лево
				Spawn(server, new Act9Wall(2, 1663, 0)); // Право
				break;
			case 2:
				Spawn(server, new Act9Wall(0, 0, 1025)); // Поталог
				Spawn(server, new Act9Wall(1, 1663, 0)); // Лево
				break;
			case 3:
				Spawn(server, new Act9Wall(0, 0, 1025)); // Поталог
				Spawn(server, new Act9Wall(2, 1663, 0)); // Право
				break;
			case 4:
				Spawn(server, new Act9Wall(1, 1663, 0)); // Лево
				Spawn(server, new Act9Wall(2, 1663, 0)); // Право
				break;
			case 5:
				Spawn(server, new Act9Wall(0, 0, 1025)); // Поталог
				break;
			case 6:
				Spawn(server, new Act9Wall(1, 1663, 0)); // Лево
				break;
			case 7:
				Spawn(server, new Act9Wall(2, 1663, 0)); // Право
				break;
			case 8:
				//Ничего
				break;
		}
		Terminal.Log($"WallRandom: {wallRandom.ToString()}"); 
		Terminal.Log($"AddTimeRandom: {addTimeRandom.ToString()}");
		base.Init(server);
	}

	public override void Tick(Server server)
	{
		base.Tick(server);
	}

	protected override int GetPlayerOffset(Server server)
	{
		lock (server.Peers)
		{
			return (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) - 1) * 10;
		}
	}

	protected override int GetRingSpawnCount()
	{
		return 38;
	}

	protected override float GetRingTime()
	{
		return 3f;
	}

	public override bool CanSpawnRedRings()
	{
		return true;
	}
}
