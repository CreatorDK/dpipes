using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Diagnostics;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test16", "Test Skip Method", "Description: Testing Skip method when run manualy and in case when OnDisconnect function is not set", true)]
    public class ServerTest16 : ServerTestBaseClass
    {
        public ServerTest16(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        BoolTrigger clientDisconnectedTrigger = new(); 

        void OnClientConnect(PacketHeader header)
        {
            Write($"1. Client Connected. Receive {header.DataSize} b. Skipping...");
            _dpipe?.Skip(header.DataSize);
            Console.WriteLine("Complete");
        }

        void OnOtherSideDisconnect(PacketHeader header)
        {
            Write($"2. Client Disconecting. Receive {header.DataSize} b. Skipping...");
            _dpipe?.Skip(header);
            Console.WriteLine("Complete");
            clientDisconnectedTrigger.SetComplete();
        }

        void OnPacketHeaderReceived(PacketHeader header)
        {
            Write($"Packet Received {header.DataSize} b. Skipping...");
            _dpipe?.Skip(header);
            Console.WriteLine("Complete");
        }

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipe = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipe == null)
                return;

            _dpipe.OnOtherSideConnectCallback = OnClientConnect;
            _dpipe.OnPacketHeaderReceivedCallback = OnPacketHeaderReceived;
            _dpipe.OnOtherSideDisconnectCallback = OnOtherSideDisconnect;

            _dpipe.StartWin32();
            var handle = _dpipe.GetHandle();
            bool processRun = RunClient(startParams, handle.AsString(), TestName);

            Trigger.Wait(clientDisconnectedTrigger);

            Thread.Sleep(500);
        }
    }
}
