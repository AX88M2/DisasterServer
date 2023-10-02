using DisasterServer.Data;
using DisasterServer.Entities;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class Act9 : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 130);
		Spawn(server, new Act9Wall(0, 0, 1025));
		Spawn(server, new Act9Wall(1, 1663, 0));
		Spawn(server, new Act9Wall(2, 1663, 0));
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
