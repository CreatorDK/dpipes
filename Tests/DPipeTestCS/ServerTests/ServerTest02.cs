using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Text;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test2", "Innitiator disconnection", "Description: Testing scenario when client connecting to existing pipe and inniciator disconnection from pipe from before client", true)]
    public class ServerTest02 : ServerTestBaseClass
    {
        public ServerTest02(string testName, string title, string description) : base(testName, title, description)  { }

        DPipe? _dpipe;
        BoolTrigger connectTrigger = new();
        BoolTrigger messageReceivedTrigger = new();

        void OnClientConnect(PacketHeader header)
        {
            WriteLine("1. Client Connected");
            WriteLine("2. Writing Greeting to Client");
            string message = "Hello, Client!";
            var data = Encoding.UTF8.GetBytes(message);
            _dpipe?.Write(data, 0, data.Length);
            connectTrigger.SetComplete();
        }

        void OnPacketHeaderReceived(PacketHeader header)
        {
            var data = new byte[header.DataSize];
            _dpipe?.Read(data, 0, data.Length);

            string message = Encoding.UTF8.GetString(data);

            if (message == "Hello, Server!")
            {
                WriteLine("3. Greeting Received");
                messageReceivedTrigger.SetComplete();
            }
        }

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipe = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipe == null)
                return;

            _dpipe.OnClientConnectCallback = OnClientConnect;
            _dpipe.OnPacketHeaderReceivedCallback = OnPacketHeaderReceived;

            //_dpipe.Start();
            _dpipe.StartWin32();

            var handle = _dpipe.GetHandle();

            bool processRun = RunClient(startParams, handle.AsString(), TestName);

            if (!processRun)
            {
                _dpipe.Disconnect();
                WriteLine("Unable to start child process");
                ReadKey();
            }

            Trigger.Wait(connectTrigger);
            Trigger.Wait(messageReceivedTrigger);
            WriteLine("4. Disconnecting From Pipe");
            _dpipe.Disconnect();
            Thread.Sleep(500);
        }
    }
}
