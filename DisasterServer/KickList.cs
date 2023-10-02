using System.Timers;
using Timer = System.Timers.Timer;

namespace DisasterServer;

public class KickList
{
	private static List<KickData> EndPoints;

	private static Timer timer;

	static KickList()
	{
		EndPoints = new List<KickData>();
		timer = new Timer();
		timer.Interval = 30000.0;
		timer.Elapsed += Timer_Elapsed;
		timer.Start();
	}

	private static void Timer_Elapsed(object? sender, ElapsedEventArgs e)
	{
		EndPoints.RemoveAll((KickData e) => (DateTime.Now - e.Since).TotalMinutes >= 1.0);
	}

	public static void Add(string endpoint)
	{
		EndPoints.Add(new KickData
		{
			IP = endpoint,
			Since = DateTime.Now
		});
	}

	public static bool Check(string endpoint)
	{
		string endpoint2 = endpoint;
		return EndPoints.Any((KickData e) => e.IP == endpoint2);
	}
}
