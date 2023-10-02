using DisasterServer.Maps;
using DisasterServer.Session;
using DisasterServer.State;

namespace DisasterServer.Entities;

public class NAPSnowball : Entity
{
	public byte ID;

	private bool _active;

	private double _frame;

	private byte _stage;

	private sbyte _dir;

	private double _stateProg;

	private double _accel;

	private float[] _waypoints;

	private float[] _waypointsSpeeds;

	private const int NUM_FRAMES = 32;

	private const int ROLL_START = 16;

	public NAPSnowball(byte nid, byte waypointCount, sbyte dir)
	{
		_accel = 0.0;
		_active = false;
		_frame = 0.0;
		_stage = 0;
		_stateProg = 0.0;
		_dir = dir;
		ID = nid;
		_waypoints = new float[waypointCount];
		Array.Fill(_waypoints, 0.05f);
		_waypointsSpeeds = new float[waypointCount];
		Array.Fill(_waypointsSpeeds, 0.35f);
	}

	public override TcpPacket? Spawn(Server server, Game game, Map map)
	{
		return null;
	}

	public override TcpPacket? Destroy(Server server, Game game, Map map)
	{
		return null;
	}

	public override UdpPacket? Tick(Server server, Game game, Map map)
	{
		if (!_active)
		{
			return null;
		}
		if (_accel > 1.0)
		{
			_frame += _waypointsSpeeds[_stage];
			if (_frame >= 32.0)
			{
				_frame = 16.0;
			}
			_stateProg += _waypoints[_stage];
		}
		else
		{
			Terminal.LogDebug($"{_accel}");
			_accel += 0.016;
			_frame += _accel * 0.44999998807907104;
			_stateProg += _accel * 0.05000000074505806;
		}
		if (_stateProg > 1.0)
		{
			_stateProg = 0.0;
			_stage++;
			if (_stage >= _waypoints.Length - 1)
			{
				_active = false;
				_stage = 0;
				_frame = 0.0;
				_stateProg = 0.0;
				server.TCPMulticast(new TcpPacket(PacketType.SERVER_NAPBALL_STATE, (byte)2, ID));
				return null;
			}
		}
		return new UdpPacket(PacketType.SERVER_NAPBALL_STATE, (byte)1, ID, _stage, (byte)_frame, _stateProg);
	}

	public void Activate(Server server)
	{
		if (!_active)
		{
			_accel = 0.0;
			_stage = 0;
			_frame = 0.0;
			_stateProg = 0.0;
			_active = true;
			server.TCPMulticast(new TcpPacket(PacketType.SERVER_NAPBALL_STATE, (byte)0, ID, _dir));
		}
	}

	public void SetWaypointMoveSpeed(byte index, float speed)
	{
		_waypoints[index] = speed;
	}

	public void SetWaypointAnimSpeed(byte index, float speed)
	{
		_waypointsSpeeds[index] = speed;
	}
}
