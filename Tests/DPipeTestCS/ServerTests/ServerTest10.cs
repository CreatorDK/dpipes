using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Text;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test10", "DPipeMessanger (Unicdoe) server - async, client - async", "Description: Testing scenario when DPipeMessanger server and client handling messages in async mode", true)]
    public class ServerTest10 : ServerTestBaseClass
    {
        public ServerTest10(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        DPMessanger? _dpipeMessanger;

        BoolTrigger connectTrigger = new();
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

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipe = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipe == null)
                return;

            _dpipeMessanger = new DPMessanger(_dpipe, Encoding.Unicode, false);
            _dpipeMessanger.OnClientConnect = OnClientConnect;
            _dpipeMessanger.OnMessageStringReceived = OnMessageStringReceived;

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

            WriteLine("7. Disconnecting From Pipe");
            _dpipeMessanger.Disconnect("I am disconnect, motherfucker!");
            Thread.Sleep(500);
        }
    }
}
