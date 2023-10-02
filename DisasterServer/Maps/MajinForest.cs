using DisasterServer.Data;
using DisasterServer.Session;

namespace DisasterServer.Maps;

public class MajinForest : Map
{
	public override void Init(Server server)
	{
		SetTime(server, 155);
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
		return 20;
	}
}
