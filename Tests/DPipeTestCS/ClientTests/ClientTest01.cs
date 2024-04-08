using CreatorDK.IO.DPipes;
using CreatorDK.Triggers;
using System.Text;
using System.Threading;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test1", "Client disconnection", "Description: Testing scenario when client connecting to existing pipe and first disconnection from it", true)]
    public class ClientTest01 : ClientTestBaseClass
    {
        public ClientTest01(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        BoolTrigger messageReceivedTrigger = new();

        void PacketHeaderReceviced(PacketHeader header)
        {
            var data = new byte[header.DataSize];
            _dpipe?.Read(data, 0, data.Length);

            var message = Encoding.UTF8.GetString(data);

            if (message == "Hello, Client!")
                WriteLine("2. Greeting Received");

            messageReceivedTrigger.SetComplete();
        }

        void WriteGreetingToServer()
        {
            string message = "Hello, Server!";

            var data = Encoding.UTF8.GetBytes(message);
            _dpipe?.Write(data, 0, data.Length);
        }

        public override void Execute(StartParamsClient startParams)
        {
            _dpipe = DPipeBuilder.Create(startParams.PipeHandle);
            if (_dpipe == null)
            {
                WriteLine("Cannot create pipe");
                ReadKey();
                return;
            }

            if (NewConsole)
                WriteTestName(_dpipe.Type);

            _dpipe.OnPacketHeaderReceivedCallback = PacketHeaderReceviced;
            WriteLine("1. Connecting to Pipe");
            _dpipe.Connect(startParams.PipeHandle);
            Trigger.Wait(messageReceivedTrigger);
            WriteLine("3. Writing Greeting to Server");
            WriteGreetingToServer();
            Thread.Sleep(100);
            WriteLine("4. Disconnecting From Pipe");

            _dpipe.Disconnect();

            if (NewConsole)
                ReadKey();
        }
    }
}
