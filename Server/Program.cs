using Server.Game;
using System;
using System.Diagnostics;
using System.Threading;

namespace Server;

internal class Program
{
    private static int _gamePid = int.MinValue;

    public static int NumServers { get; private set; } = 1;

    public static int Port { get; private set; } = 4089;

    private static void Main(string[] args)
    {
        if (args.Length != 0)
        {
            for (int i = 0; i < args.Length; i++)
            {
                switch (args[i])
                {
                case "--help":
                case "--h":
                case "-h":
                case "--?":
                case "-?":
                case "?":
                    Console.WriteLine(AppDomain.CurrentDomain.FriendlyName + " [options...]");
                    Console.WriteLine("(c) 2023 Team Exe Empire");
                    Console.WriteLine("\nOptions");
                    Console.WriteLine("--help: Show this screen");
                    Console.WriteLine("--version: Prints server's BUILD_VERSION");
                    Console.WriteLine("--nservers [number]: Specifies number of servers to start.   (default 1)");
                    Console.WriteLine("--port [number]: Specifies port of the server.               (default 4089)");
                    return;
                case "--version":
                case "--ver":
                case "--v":
                    Console.WriteLine(206);
                    return;
                case "--nservers":
                    if (i + 1 >= args.Length)
                    {
                        Console.WriteLine("Please specify valid number.");
                        return;
                    }
                    try
                    {
                        NumServers = Convert.ToInt32(args[i + 1]);
                        if (NumServers >= 99)
                        {
                            throw new FormatException();
                        }
                    }
                    catch (FormatException)
                    {
                        Console.WriteLine("ur mad");
                    }
                    break;
                case "--port":
                    if (i + 1 >= args.Length)
                    {
                        Console.WriteLine("Please specify valid number in range [1024..65536].");
                        return;
                    }
                    try
                    {
                        Port = Convert.ToInt32(args[i + 1]);
                        if (Port < 1024 || NumServers >= Port)
                        {
                            throw new FormatException();
                        }
                    }
                    catch (FormatException)
                    {
                        Console.WriteLine("Please specify valid number in range [1024..65536].");
                    }
                    break;
                case "--gamePID":
                    if (i + 1 >= args.Length)
                    {
                        Console.WriteLine("why are you passing this value again?");
                        return;
                    }
                    try
                    {
                        _gamePid = Convert.ToInt32(args[i + 1]);
                    }
                    catch (FormatException)
                    {
                        Console.WriteLine("baka do you even know what youre doing, please dont touch this :eye:");
                    }
                    break;
                }
            }
        }

        Logger.Log("=============================");
        Logger.Log(" SeTD2R Server ");
        Logger.Log($" BUILD v{206}");
        Logger.Log(" (c) 2023 Team Exe Empire ");
        Logger.Log("=============================\n");

        for (int j = 0; j < NumServers; j++)
        {
            new Server.Game.Server(j + 1, Port + j).Start();
        }
        while (true)
        {
            Thread.Sleep(1000);
            if (_gamePid != int.MinValue)
            {
                try
                {
                    Process.GetProcessById(_gamePid);
                }
                catch
                {
                    break;
                }
            }
        }
    }
}