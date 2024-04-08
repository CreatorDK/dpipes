using CreatorDK.IO.DPipes;
using System.Diagnostics;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test17", "Named Pipe Remote", "Description: Testing Named Pipe Connection over Local Network", false)]
    public class ClientTest17 : ClientTestBaseClass
    {
        public ClientTest17(string testName, string title, string description) : base(testName, title, description) { }

        DPipeNamed? _dpipeNamed;

        public override void Execute(StartParamsClient startParams)
        {
            _dpipeNamed = DPipeNamed.Create(startParams.PipeHandle);
            if (_dpipeNamed == null)
            {
                WriteLine("Cannot create named pipe");
                ReadKey();
                return;
            }

            if (NewConsole)
                WriteTestName(_dpipeNamed.Type);

            WriteLine("Waiting wor debugger attached");
            ReadKey();

            WriteLine("1. Connecting");

            try
            {
                _dpipeNamed.Connect(startParams.PipeHandle);
                WriteLine("2. Connected!");
            }
            catch (Exception ex)
            {
                WriteLine($"Exception at connection {ex}");
            }

            if (NewConsole)
                ReadKey();
        }
    }
}
