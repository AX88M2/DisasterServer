using DisasterServer.Data;

namespace DisasterServer.UI;

public abstract class Window
{
	public abstract bool Run();

	public abstract void Log(string message);

	public virtual void AddPlayer(Peer peer)
	{
	}

	public virtual void RemovePlayer(Peer peer)
	{
	}
}
