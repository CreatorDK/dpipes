using CreatorDK.IO.DPipes;
using System.Text;
using CreatorDK.DTE;
using CreatorDK.Diagnostics.Win32;

namespace DPipeTestCS
{
    public abstract class ServerTestBaseClass
    {
        public bool NewConsole {  get; private set; }
        public string TestName { get; init; }
        public string Title { get; init; }
        public string Description { get; init; }

        protected ServerTestBaseClass(string testName, string title, string description) 
        {
            TestName = testName;
            Title = title;
            Description = description;
        }

        public void WriteTestName(DP_TYPE pipeType)
        {
            if (pipeType == DP_TYPE.ANONYMOUS_PIPE)
                Console.WriteLine("Anonymus Pipe Server");

            else
                Console.WriteLine("Named Pipe Server");

            Console.WriteLine($"{TestName} ({Title})");
            Console.WriteLine(Description);
        }
        public static void Write(string? line)
        {
            Console.Write($"CS SRV: {line}");
        }
        public static void WriteLine(string? line)
        {
            Console.WriteLine($"CS SRV: {line}");
        }

        public static void ReadKey()
        {
            Console.WriteLine("\nPress any key to continue...");
            Console.ReadKey();
        }

        public static string GetArgs(StartParamsServer parameters, string path, string handle, string testName, bool includePath)
        {
            path = $"\"{path}\"";
            StringBuilder sb = new();

            if (includePath)
                sb.Append($"{path} ");

            sb.Append($"client {handle} {testName}");

            foreach (var pair in parameters.Flags)
            {
                if (pair.Key == "/path")
                    continue;
                else if (pair.Key == "/attach_client")
                {
                    sb.Append(" /attach");
                    continue;
                }
                else if (!string.IsNullOrEmpty(pair.Value))
                    sb.Append($" {pair.Key} {pair.Value}");
                else
                    sb.Append($" {pair.Key}");
            }

            return sb.ToString();
        }

        public static bool RunClientWin32(string path, string args, bool attachChildProcess) {

            ProcessWin32 process = new(path, args)
            {
                CreationFlags = CreationFlags.CREATE_NEW_CONSOLE,
                InheritHandles = true
            };

            bool result = process.Start();

            if (result && attachChildProcess)
            {
                var processId = process.Id;
                var processDTE = ProcessDTE.GetProcess(processId);
                processDTE?.Attach();
            }

            return result;
        }

        public static bool RunClient(StartParamsServer parameters, string handle, string testName)
        {
            string path = parameters.Path.Replace(".dll", ".exe");
            bool attachChildProcess = false;

            foreach (var pair in parameters.Flags)
            {
                if (pair.Key == "/path" && pair.Value.Length > 0)
                    path = pair.Value;
                else if (pair.Key == "/attach_client")
                    attachChildProcess = true;
            }

            if (parameters.NewConsole)
                return RunClientWin32(path, GetArgs(parameters, path, handle, testName, true), attachChildProcess);

            System.Diagnostics.Process process = new();
            process.StartInfo.FileName = path;
            process.StartInfo.Arguments = GetArgs(parameters, path, handle, testName, false);
            process.StartInfo.UseShellExecute = false;
            bool result = process.Start();

            if (result && attachChildProcess)
            {
                var processId = process.Id;
                var processDTE = ProcessDTE.GetProcess(processId);
                processDTE?.Attach();
            }

            return result;
        }

        public void ExecuteWrapper(StartParamsServer startParams)
        {
            NewConsole = startParams.NewConsole;
            Execute(startParams);
        }

        public abstract void Execute(StartParamsServer startParams);
    }
}
