using System;
using System.Net;

namespace Server.Game;

public class Player
{
	public Guid GID;

	public EndPoint EndPoint;

	public ushort ID;

	public string Nickname = "Pending...";

	public Character Character = Character.NONE;

	public int DeadTimer = -1;

	public byte RevivalTimes;

	public bool Pending = true;

	public bool IsReady;

	public bool IsAlive = true;

	public bool DiedBefore;

	public bool HasEscaped;

	public bool CanDemonize;

	public int LastPacketTime;
}
