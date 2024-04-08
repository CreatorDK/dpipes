namespace DPipeTestCS
{
    public class StartParamsClient
    {
        private StartParamsClient(string testName, string pipeHandle, bool newConsole, Dictionary<string, string> flags) 
        {
            TestName = testName;
            PipeHandle = pipeHandle;
            Flags = flags;
            NewConsole = newConsole;
        }
        public bool NewConsole { get; init; }
        public string TestName { get; init; }
        public string PipeHandle { get; init; }
        public Dictionary<string, string> Flags { get; init; }

        public static StartParamsClient Create(string[] args)
        {
            if (args.Length < 3)
                throw new ArgumentException("Not enough arguments");

            string testName = "all";

            Dictionary<string, string> flags;

            if (args.Length >= 4)
            {
                if (!args[3].StartsWith('/'))
                {
                    testName = args[3];
                    flags = Programm.GetFlags(args, 4);
                }
                else
                {
                    flags = Programm.GetFlags(args, 3);
                }
            }
            else
            {
                flags = [];
            }

            bool newCosole = false;

            foreach (var pair in flags)
            {
                if (pair.Key == "/new")
                    newCosole = true;
            }

            return new StartParamsClient(testName, args[2], newCosole, flags);
        }
    }
}
