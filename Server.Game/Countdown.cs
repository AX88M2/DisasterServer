using System;
using System.Timers;

namespace Server.Game;

public class Countdown
{
	private bool _counting;

	private Timer _timer = new Timer();

	public int Count;

	public bool IsCounting => _counting;

	public event Action<int> CountEvent;

	public event Action ReachedEvent;

	public Countdown()
	{
		Count = 5;
		lock (_timer)
		{
			_timer.Interval = 1000.0;
			_timer.Elapsed += _timer_Elapsed;
		}
	}

	private void _timer_Elapsed(object sender, ElapsedEventArgs e)
	{
		Count--;
		this.CountEvent?.Invoke(Count);
		if (Count == 0)
		{
			lock (_timer)
			{
				_timer.Stop();
			}
			_counting = false;
			this.ReachedEvent?.Invoke();
		}
	}

	public void Start()
	{
		lock (_timer)
		{
			_timer.Start();
		}
		_counting = true;
	}

	public void Stop()
	{
		lock (_timer)
		{
			_timer.Stop();
		}
		_counting = false;
	}
}
