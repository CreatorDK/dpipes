using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test17", "Named Pipe Remote", "Description: Testing Named Pipe Connection over Local Network", false)]
    public class ServerTest17 : ServerTestBaseClass
    {
        public ServerTest17(string testName, string title, string description) : base(testName, title, description) { }

        DPNamed? _dpipeNamed;
        BoolTrigger clientConnectedTrigger = new(); 

        void OnClientConnect(PacketHeader header)
        {
            Write("1. Client Connected");
            clientConnectedTrigger.SetComplete();
        }

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipeNamed = DPNamed.Create("\\\\.\\pipe\\test-pipe-123");

            if (_dpipeNamed == null)
                return;

            _dpipeNamed.OnOtherSideConnectCallback = OnClientConnect;
            _dpipeNamed.StartWin32();

            _dpipeNamed.UseRemote = true;

            var handleString = _dpipeNamed.GetHandleString();

            WriteLine($"Pipe handle: {handleString}");
            WriteLine("Waiting for the clinet connect");

            Trigger.Wait(clientConnectedTrigger);
        }
    }
}
