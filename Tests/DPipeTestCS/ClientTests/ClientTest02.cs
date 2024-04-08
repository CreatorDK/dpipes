using CreatorDK.IO.DPipes;
using CreatorDK.Triggers;
using System.Text;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test2", "Innitiator disconnection", "Description: Testing scenario when client connecting to existing pipe and inniciator disconnection from pipe from before client", true)]
    public class ClientTest02 : ClientTestBaseClass
    {
        public ClientTest02(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        BoolTrigger messageReceivedTrigger = new();
        BoolTrigger disconnectTrigger = new();

        void PacketHeaderReceviced(PacketHeader header)
        {
            var data = new byte[header.DataSize];
            _dpipe?.Read(data, 0, data.Length);

            var message = Encoding.UTF8.GetString(data);

            if (message == "Hello, Client!")
                WriteLine("2. Greeting Received");

            messageReceivedTrigger.SetComplete();
        }

        void OnOtherSideDisconnect(PacketHeader header)
        {
            WriteLine("4. Server Disconecting");
            disconnectTrigger.SetComplete();
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
            _dpipe.OnOtherSideDisconnectCallback = OnOtherSideDisconnect;
            WriteLine("1. Connecting to Pipe");
            _dpipe.Connect(startParams.PipeHandle);
            Trigger.Wait(messageReceivedTrigger);
            WriteLine("3. Writing Greeting to Server");
            WriteGreetingToServer();
            Thread.Sleep(100);
            Trigger.Wait(disconnectTrigger);

            if (NewConsole)
                ReadKey();
        }
    }
}
