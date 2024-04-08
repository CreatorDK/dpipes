using CreatorDK.IO.DPipes;
using CreatorDK.Triggers;
using System.Text;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test8", "DPipeMessanger (Unicode) server - sync, client - async", "Description: Testing scenario when DPipeMessanger server in sync mode and client in async mode", true)]
    public class ClientTest08 : ClientTestBaseClass
    {
        public ClientTest08(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        DPipeMessanger? _dpipeMessanger;

        BoolTrigger disconnectTrigger = new();
        BoolTrigger messageReceivedTrigger = new();
        BoolTrigger received10MessagesSyncTrigger = new();
        BoolTrigger received10MessagesAsyncTrigger = new();

        void WriteFrom10ThreadsSync()
        {
            for (int i = 0; i < 10; i++)
            {
                string message = $"Creating thread (Send sync) {i}";
                if (NewConsole)
                    WriteLine(message);
                _dpipeMessanger?.SendMessage(message);
            }
        }

        void WriteFrom10ThreadsASync()
        {
            for (int i = 0; i < 10; i++)
            {
                string message = $"Creating thread (Send sync) {i}";
                if (NewConsole)
                    WriteLine(message);
                _dpipeMessanger?.SendMessageAsync(message);
            }
        }

        void OnMessageStringReceived(string? message)
        {
            if (message == "Hello, Client!")
            {
                WriteLine("2. Greeting Received");
                messageReceivedTrigger.SetComplete();
            }
            else if (message == "MessagesReceivedSync!")
                received10MessagesSyncTrigger.SetComplete();
            else if (message == "MessagesReceivedAsync!")
                received10MessagesAsyncTrigger.SetComplete();
        }

        void OnOtherSideDisconnect(string? message)
        {
            WriteLine($"7. Server Disconecting with message: {message}");
            disconnectTrigger.SetComplete();
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

            _dpipeMessanger = new DPipeMessanger(_dpipe, Encoding.Unicode, true);
            _dpipeMessanger.OnMessageStringReceived = OnMessageStringReceived;
            _dpipeMessanger.OnOtherSideDisconect = OnOtherSideDisconnect;
            WriteLine("1. Connecting to Pipe");
            _dpipeMessanger.Connect(startParams.PipeHandle, "I am connect, motherfucker!");
            Trigger.Wait(messageReceivedTrigger);

            WriteLine("3. Sending message from 10 thread sync");
            WriteFrom10ThreadsSync();

            Write("4. Waiting confirmation...");
            Trigger.Wait(received10MessagesSyncTrigger);
            Console.WriteLine("Complete");

            WriteLine("5. Sending message from 10 thread async");
            WriteFrom10ThreadsASync();
            Write("6. Waiting confirmation...");
            Trigger.Wait(received10MessagesAsyncTrigger);
            Console.WriteLine("Complete");

            Trigger.Wait(disconnectTrigger);
            _dpipeMessanger.Disconnect();

            if (NewConsole)
                ReadKey();
        }
    }
}
