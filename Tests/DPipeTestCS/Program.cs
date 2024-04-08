using System;
using System.Collections.Generic;
using System.IO.Pipes;
using System.Threading;

namespace DPipeTestCS
{
    static class Programm
    {
        static int Main()
        {
            string[] args = Environment.GetCommandLineArgs();

            foreach (var arg in args)
            {
                if (arg == "/attach")
                {
                    Thread.Sleep(3000);
                    break;
                }
            }

            if (args.Length < 3)
            {
                Console.WriteLine("Not enough arguments for start tests");
                Console.ReadLine();
                return 1;
            }

            if (args[1] == "server")
            {
                try
                {
                    var servervParams = StartParamsServer.Create(args);

                    if (args[2] == "all")
                    {
                        ServerTest.Run(servervParams, true);
                    }
                    else
                    {
                        ServerTest.Run(servervParams, false);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred while getting startup server parameters: {ex.Message}");
                }
            }
            else if (args[1] == "client")
            {
                try
                {
                    var clietParams = StartParamsClient.Create(args);
                    ClientTest.Run(clietParams);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"Exception occurred while getting startup client parameters: {ex.Message}");
                }
            }

            else
            {
                Console.WriteLine("Unkown role");
                Console.ReadLine();
                return 2;
            }

            return 0;
        }

        public static Dictionary<string, string> GetFlags(string[] args, int startIndex)
        {
            Dictionary<string, string> result = [];

            int argsCount = args.Length;

            if (argsCount < startIndex)
                return result;

            for (int i = startIndex; i < argsCount; i++)
            {
                if (args[i].StartsWith('/'))
                {
                    string argKey = args[i];
                    string argValue = string.Empty;

                    int nextIndex = i + 1;

                    if (nextIndex < argsCount)
                    {
                        if (!args[nextIndex].StartsWith('/'))
                        {
                            argValue = args[nextIndex];
                            ++i;
                        }
                    }

                    result.Add(argKey, argValue);
                }

            }

            return result;
        }
    }
}

