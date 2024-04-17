using CreatorDK.IO.DPipes;
using CreatorDK.Triggers;
using System.Text;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test14", "DPServer - async, DPClient - async", "Description: Testing DPServer and DPClient communication", true)]
    public class ClientTest14 : ClientTestBaseClass
    {
        public ClientTest14(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        DPClient _dpclient;

        BoolTrigger messagesReceivedTrigger = new();
        BoolTrigger resp4ReceivedTrigger = new();
        BoolTrigger resp5ReceivedTrigger = new();
        BoolTrigger resp6ReceivedTrigger = new();
        IntTrigger send10SyncRequestTrigger = new(10);
        IntTrigger send10AsyncRequestTrigger = new(10);


        void OnInfoStringReceived(string? message)
        {
            if (message == "Info string from server")
                WriteLine("8. Info string received");
        }

        void OnInfoDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0)
            {
                string message = _dpclient.Encoding.GetString(data);
                if (message == "Info data from server")
                    WriteLine("9. Info data received");
            }
        }

        void OnWarningStringReceived(string? message)
        {
            if (message == "Warning string from server")
                WriteLine("10. Warning string received");
        }

        void OnWarningDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0)
            {
                string message = _dpclient.Encoding.GetString(data);
                if (message == "Warning data from server")
                    WriteLine("11. Warning data received");
            }
        }

        void OnErrorStringReceived(string? message)
        {
            if (message == "Error string from server")
                WriteLine("12. Error string received");
        }

        void OnErrorDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0)
            {
                string message = _dpclient.Encoding.GetString(data);
                if (message == "Error data from server")
                    WriteLine("13. Error data received");

                messagesReceivedTrigger.SetComplete();
            }
        }

        private void Repsponse4Callback(Task<DPResponse> task)
        {
            var resp = task.Result;
            WriteLine($"17. Received response callback 4 in {resp.TotalDuraction.ToString(@"ss\.fff")}s");
            resp4ReceivedTrigger.SetComplete();
        }

        private void Repsponse5Callback(Task<DPResponse> task)
        {
            var resp = task.Result;
            WriteLine($"18. Received response callback 5 in {resp.TotalDuraction.ToString(@"ss\.fff")}s");
            resp5ReceivedTrigger.SetComplete();
        }

        private void Repsponse6Callback(Task<DPResponse> task)
        {
            var resp = task.Result;
            WriteLine($"19. Received response callback 6 in {resp.TotalDuraction.ToString(@"ss\.fff")}s");
            resp6ReceivedTrigger.SetComplete();
        }

        private void Send10RequestSync()
        {
            for (int i = 0; i < 10; i++)
            {
                Task.Run(() =>
                {
                    var message = _dpclient.Encoding.GetBytes($"Requset from {Thread.CurrentThread.ManagedThreadId}");
                    if (NewConsole)
                        WriteLine($"Request send sync in thread {Thread.CurrentThread.ManagedThreadId}");

                    _dpclient.SendAsync(7, 8, message);
                }).Wait();

                send10SyncRequestTrigger.Increase(1);
            }
        }

        private void Send10RequestAsync()
        {
            for (int i = 0; i < 10; i++)
            {
                Task.Run(() =>
                {
                    var message = _dpclient.Encoding.GetBytes($"Requset from {Thread.CurrentThread.ManagedThreadId}");
                    if (NewConsole)
                        WriteLine($"Request send async in thread {Thread.CurrentThread.ManagedThreadId}");

                    _dpclient.SendRequest(7, 8, message);
                });

                send10AsyncRequestTrigger.Increase(1);
            }
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

            _dpclient = new DPClient(_dpipe, true, Encoding.Unicode);
            _dpclient.OnInfoStringReceived = OnInfoStringReceived;
            _dpclient.OnInfoDataReceived = OnInfoDataReceived;
            _dpclient.OnWarningStringReceived = OnWarningStringReceived;
            _dpclient.OnWarningDataReceived = OnWarningDataReceived;
            _dpclient.OnErrorStringReceived = OnErrorStringReceived;
            _dpclient.OnErrorDataReceived = OnErrorDataReceived;

            WriteLine("1. Connecting to server");
            _dpclient.Connect(startParams.PipeHandle, "I am connect, motherfucker!");

            WriteLine("2. Sending Info String");
            _dpclient.SendInfo("Info string from client");

            WriteLine("3. Sending Info Data");
            byte[]? infoData = _dpclient.Encoding.GetBytes("Info data from client");
            _dpclient.SendInfo(infoData);

            WriteLine("4. Sending Warning String");
            _dpclient.SendWarning("Warning string from client");

            WriteLine("5. Sending Warning Data");
            byte[]? warningData = _dpclient.Encoding.GetBytes("Warning data from client");
            _dpclient.SendWarning(warningData);

            WriteLine("6. Sending Error String");
            _dpclient.SendError("Error string from client");

            WriteLine("7. Sending Error Data");
            byte[]? errorData = _dpclient.Encoding.GetBytes("Info data from client");
            _dpclient.SendError(errorData);

            Trigger.Wait(messagesReceivedTrigger);

            var message1 = _dpclient.Encoding.GetBytes("Hello handler 1!");
            var resp1 = _dpclient.SendRequest(1, 2, message1);
            WriteLine($"14. Received response 1 in {resp1.TotalDuraction.ToString(@"ss\.fff")}s");

            var message2 = _dpclient.Encoding.GetBytes("Hello handler 2!");
            var resp2 = _dpclient.SendRequest(2, 3, message2);
            WriteLine($"15. Received response 2 in {resp2.TotalDuraction.ToString(@"ss\.fff")}s");

            var message3 = _dpclient.Encoding.GetBytes("Hello handler 3!");
            var resp3 = _dpclient.SendRequest(3, 4, message3);
            WriteLine($"16. Received response 3 in {resp3.TotalDuraction.ToString(@"ss\.fff")}s");

            var message4 = _dpclient.Encoding.GetBytes("Hello handler 4!");
            _dpclient.SendAsync(4, 5, message4).ContinueWith(Repsponse4Callback);

            var message5 = _dpclient.Encoding.GetBytes("Hello handler 5!");
            _dpclient.SendAsync(5, 6, message5).ContinueWith(Repsponse5Callback);

            var message6 = _dpclient.Encoding.GetBytes("Hello handler 6!");
            _dpclient.SendAsync(6, 7, message6).ContinueWith(Repsponse6Callback);

            Send10RequestSync();
            Trigger.Wait(send10SyncRequestTrigger);

            Send10RequestAsync();
            Trigger.Wait(send10AsyncRequestTrigger);

            Trigger.Wait(resp4ReceivedTrigger);
            Trigger.Wait(resp5ReceivedTrigger);
            Trigger.Wait(resp6ReceivedTrigger);

            WriteLine("20. Sending last request");
            var messageLast = _dpclient.Encoding.GetBytes("Hello last handler!");
            _dpclient.SendRequest(9, 10, messageLast);

            WriteLine("21. Disconnecting");
            _dpclient.Disconnect("Goodbuy, motherfucker!");

            if (NewConsole)
                ReadKey();
        }
    }
}
