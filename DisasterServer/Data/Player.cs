namespace DisasterServer.Data;

public class Player
{
	public Character Character = Character.None;

	public ExeCharacter ExeCharacter = ExeCharacter.None;

	public int RevivalTimes;

	public float DeadTimer = -1f;

	public bool DiedBefore;

	public bool HasEscaped;

	public bool IsReady;

	public bool IsAlive = true;

	public bool IsHurt;

	public bool HasRedRing;

	public bool CanDemonize;

	public bool Invisible;

	public float LossCount;

	public long LastLostTime;

	public float X;

	public float Y;

	public float BacktrackX;

	public float BacktrackY;

	public bool Backtrack;

	public int BacktrackTick;
}
