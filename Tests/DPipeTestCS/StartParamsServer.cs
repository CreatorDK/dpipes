using CreatorDK.IO.DPipes;

namespace DPipeTestCS
{
    public class StartParamsServer
    {
        private StartParamsServer(string path, DPIPE_TYPE pipeType, string testName, bool newConsole, Dictionary<string, string> flags) 
        {
            Path = path;
            PIPE_TYPE = pipeType;
            TestName = testName;
            Flags = flags;
            NewConsole = newConsole;
        }

        public bool NewConsole { get; init; }

        public string Path { get; init; }

        public DPIPE_TYPE PIPE_TYPE { get; set; }

        public string TestName { get; init; }

        public Dictionary<string, string> Flags { get; init; }

        public static StartParamsServer Create(string[] args)
        {
            if (args.Length < 3)
                throw new ArgumentException("Not enough arguments");

            var pipeType = args[2] switch
            {
                "anonymus" => DPIPE_TYPE.ANONYMUS_PIPE,
                "named" => DPIPE_TYPE.NAMED_PIPE,
                "all" => DPIPE_TYPE.ANONYMUS_PIPE,
                _ => throw new ArgumentException("Unknown pipeType argument"),
            };
            Dictionary<string, string> flags;

            string testName = "all";

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

            return new StartParamsServer(args[0], pipeType, testName, newCosole, flags);
        }
    }
}
