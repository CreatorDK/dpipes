using CreatorDK.IO.DPipes;
using System;

namespace DPipeTestCS
{
    public abstract class ClientTestBaseClass
    {
        public bool NewConsole { get; private set; }
        public string TestName { get; init; }
        public string Title { get; init; }
        public string Description { get; init; }

        protected ClientTestBaseClass(string testName, string title, string description)
        {
            TestName = testName;
            Title = title;
            Description = description;
        }

        public void WriteTestName(DPIPE_TYPE pipeType)
        {
            if (pipeType == DPIPE_TYPE.ANONYMUS_PIPE)
                Console.WriteLine("Anonymus Pipe Client");

            else
                Console.WriteLine("Named Pipe Client");

            Console.WriteLine($"{TestName} ({Title})");
            Console.WriteLine(Description);
        }

        public static void WriteLine(string line)
        {
            Console.WriteLine($"CS CL: {line}");
        }

        public static void Write(string line)
        {
            Console.Write($"CS CL: {line}");
        }

        public static void ReadKey()
        {
            Console.WriteLine("\nPress any key to continue...");
            Console.ReadKey();
        }

        public static void WriteDisconnectionLine(DPIPE_TYPE pipeType)
        {
            string pipeTypeString;
            if (pipeType == DPIPE_TYPE.ANONYMUS_PIPE)
                pipeTypeString = "ANONYMUS";
            else
                pipeTypeString = "NAMED";

            Console.WriteLine($"CLIENT {pipeTypeString} Disconnected");
        }

        public void ExecuteWrapper(StartParamsClient startParams)
        {
            NewConsole = startParams.NewConsole;
            Execute(startParams);
        }

        public abstract void Execute(StartParamsClient startParams);
    }
}
