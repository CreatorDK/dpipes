using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Text;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test9", "DPipeMessanger (Unicode) server - async, client - sync", "Description: Testing scenario when DPipeMessanger server in async mode and client in sync mode", true)]
    public class ServerTest09 : ServerTestBaseClass
    {
        public ServerTest09(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        DPipeMessanger? _dpipeMessanger;

        BoolTrigger connectTrigger = new();
        BoolTrigger disconnectTrigger = new();
        IntTrigger messageSyncReceivedTrigger = new(10);
        IntTrigger messageAsyncReceivedTrigger = new(10);

        void Send10MessagesReceivedSyncConfirmation()
        {
            _dpipeMessanger?.SendMessage("MessagesReceivedSync!");
        }

        void Send10MessagesReceivedAsyncConfirmation()
        {
            _dpipeMessanger?.SendMessage("MessagesReceivedAsync!");
        }

        void OnClientConnect(string? onConnectMessage)
        {
            WriteLine($"1. Client Connected with message: {onConnectMessage}");
            WriteLine("2. Sending Greeting to client");
            _dpipeMessanger?.SendMessage("Hello, Client!");
            connectTrigger.SetComplete();
        }

        void OnMessageStringReceived(string? message)
        {
            messageSyncReceivedTrigger.Increase(1);

            if (messageSyncReceivedTrigger.IsComplete())
                messageAsyncReceivedTrigger.Increase(1);

            if (NewConsole)
                WriteLine(message);
        }

        void OnClientDisconnect(string? onDisonnectMessage)
        {
            WriteLine($"7. Client Disconecting with message: {onDisonnectMessage}");
            disconnectTrigger.SetComplete();
        }

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipe = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipe == null)
                return;

            _dpipeMessanger = new DPipeMessanger(_dpipe, Encoding.Unicode, true);
            _dpipeMessanger.OnClientConnect = OnClientConnect;
            _dpipeMessanger.OnMessageStringReceived = OnMessageStringReceived;
            _dpipeMessanger.OnOtherSideDisconect = OnClientDisconnect;

            //_dpipe.Start();
            _dpipe.StartWin32();

            var handle = _dpipe.GetHandle();

            bool processRun = RunClient(startParams, handle.AsString(), TestName);
            Trigger.Wait(connectTrigger);

            WriteLine("3. Waiting to receive sync sended strings from 10 threads");
            Trigger.Wait(messageSyncReceivedTrigger);

            WriteLine("4. Sending 10 messages recieved sync confirmation");
            Send10MessagesReceivedSyncConfirmation();

            WriteLine("5. Waiting to receive async sended strings from 10 threads");
            Trigger.Wait(messageAsyncReceivedTrigger);

            WriteLine("6. Sending 10 messages recieved async confirmation");
            Send10MessagesReceivedAsyncConfirmation();

            Trigger.Wait(disconnectTrigger);
            _dpipe.Disconnect();
            Thread.Sleep(500);
        }
    }
}
