using System.Net;
using DisasterServer.Data;
using DisasterServer.Maps;
using DisasterServer.Session;
using ExeNet;

namespace DisasterServer.State;

public class MapVote : State
{
	public static readonly Type[] Maps = new Type[19]
	{
		typeof(HideAndSeek2),
		typeof(RavineMist),
		typeof(DotDotDot),
		typeof(DesertTown),
		typeof(YouCantRun),
		typeof(LimpCity),
		typeof(NotPerfect),
		typeof(KindAndFair),
		typeof(Act9),
		typeof(NastyParadise),
		typeof(PricelessFreedom),
		typeof(VolcanoValley),
		typeof(GreenHill),
		typeof(MajinForest),
		typeof(AngelIsland),
		typeof(TortureCave),
		typeof(DarkTower),
		typeof(HauntingDream),
		typeof(FartZone)
	};

	public static List<int> Excluded = new List<int> { 18 };

	private MapVoteMap[] _votes = new MapVoteMap[3]
	{
		new MapVoteMap(),
		new MapVoteMap(),
		new MapVoteMap()
	};

	private int _timer = 60;

	private int _timerSec = 30;

	private Random _rand = new Random();

	private Dictionary<ushort, bool> _votePeers = new Dictionary<ushort, bool>();

	public override DisasterServer.Session.State AsState()
	{
		return DisasterServer.Session.State.VOTE;
	}

	public override void Init(Server server)
	{
		List<int> numbers = new List<int>();
		int number = _rand.Next(0, Maps.Length);
		int uniqueCount = 0;
		for (int l = 0; l < Maps.Length; l++)
		{
			if (!Excluded.Contains(l))
			{
				uniqueCount++;
			}
		}
		if (uniqueCount <= 3)
		{
			server.LastMap = -1;
		}
		for (int k = 0; k < ((uniqueCount >= _votes.Length) ? _votes.Length : uniqueCount); k++)
		{
			while (Excluded.Contains(number) || numbers.Contains(number) || number == server.LastMap)
			{
				number = _rand.Next(0, Maps.Length - 1);
			}
			numbers.Add(number);
		}
		if (uniqueCount < _votes.Length)
		{
			for (int j = 0; j < _votes.Length - uniqueCount; j++)
			{
				numbers.Add(number);
			}
		}
		for (int i = 0; i < numbers.Count; i++)
		{
			_votes[i].Map = Ext.CreateOfType<Map>(Maps[numbers[i]]) ?? new HideAndSeek2();
			_votes[i].MapID = (byte)numbers[i];
			_votes[i].Votes = 0;
		}
		lock (server.Peers)
		{
			foreach (KeyValuePair<ushort, Peer> peer in server.Peers)
			{
				if (peer.Value.Pending)
				{
					server.DisconnectWithReason(server.GetSession(peer.Key), "PacketType.CLIENT_REQUESTED_INFO missing.");
				}
				else if (!peer.Value.Waiting)
				{
					_votePeers.Add(peer.Key, value: false);
				}
			}
		}
		TcpPacket packet = new TcpPacket(PacketType.SERVER_VOTE_MAPS, _votes[0].MapID, _votes[1].MapID, _votes[2].MapID);
		server.TCPMulticast(packet);
		Program.Stat?.MulticastInformation();
	}

	public override void PeerJoined(Server server, TcpSession session, Peer peer)
	{
	}

	public override void PeerLeft(Server server, TcpSession session, Peer peer)
	{
		lock (server.Peers)
		{
			if (server.Peers.Count<KeyValuePair<ushort, Peer>>((KeyValuePair<ushort, Peer> e) => !e.Value.Waiting) <= 1)
			{
				server.SetState<Lobby>();
				return;
			}
			lock (_votePeers)
			{
				_votePeers.Remove(session.ID);
				if (_votePeers.Count((KeyValuePair<ushort, bool> e) => !e.Value) <= 0)
				{
					CheckVotes(server);
				}
			}
		}
	}

	public override void PeerTCPMessage(Server server, TcpSession session, BinaryReader reader)
	{
		bool num = reader.ReadBoolean();
		byte type = reader.ReadByte();
		if (num)
		{
			server.Passtrough(reader, session);
		}
		switch ((PacketType)type)
		{
		case PacketType.IDENTITY:
			Ext.HandleIdentity(server, session, reader);
			break;
		case PacketType.CLIENT_VOTE_REQUEST:
		{
			byte map = reader.ReadByte();
			if (map >= _votes.Length || _votePeers[session.ID])
			{
				break;
			}
			lock (_votePeers)
			{
				_votePeers[session.ID] = true;
				if (_votePeers.Count((KeyValuePair<ushort, bool> e) => !e.Value) <= 0 && _timerSec > 3)
				{
					_timer = 1;
					_timerSec = 4;
				}
			}
			_votes[map].Votes++;
			TcpPacket pkt = new TcpPacket(PacketType.SERVER_VOTE_SET, (byte)_votes[0].Votes, (byte)_votes[1].Votes, (byte)_votes[2].Votes);
			server.TCPMulticast(pkt);
			break;
		}
		}
	}

	public override void PeerUDPMessage(Server server, IPEndPoint IPEndPoint, ref byte[] data)
	{
	}

	public override void Tick(Server server)
	{
		if (_timer-- <= 0)
		{
			_timer = 60;
			_timerSec--;
			TcpPacket packet = new TcpPacket(PacketType.SERVER_VOTE_TIME_SYNC, (byte)_timerSec);
			server.TCPMulticast(packet);
			if (_timerSec <= 0)
			{
				CheckVotes(server);
			}
		}
	}

	private void CheckVotes(Server server)
	{
		int max = _votes.Max((MapVoteMap e) => e.Votes);
		MapVoteMap[] votes = _votes.Where((MapVoteMap e) => e.Votes == max).ToArray();
		MapVoteMap map = votes[_rand.Next(0, votes.Length)];
		server.LastMap = map.MapID;
		server.SetState(new CharacterSelect(map.Map));
	}
}
