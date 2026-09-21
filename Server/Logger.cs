using System;

namespace Server
{
    internal class Logger
    {
        public static void Log(string text, ConsoleColor color = ConsoleColor.Gray, int serverId = -1)
        {
            Console.ForegroundColor = color;
            Console.WriteLine("[Server " + DateTime.Now.ToLongTimeString() + "] " + text);
            Console.ResetColor();
        }
    }
}