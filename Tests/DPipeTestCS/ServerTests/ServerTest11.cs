using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Reflection.PortableExecutable;
using System.Text;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test11", "DPServer - sync, DPClient - sync", "Description: Testing DPServer and DPClient communication", true)]
    public class ServerTest11 : ServerTestBaseClass
    {
        public ServerTest11(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        DPServer? _dpserver;

        BoolTrigger lastRequestTrigger = new();
        BoolTrigger messagesReceivedTrigger = new();

        void OnClientConnect(PacketHeader header)
        {
            if (header.DataSize > 0 && _dpserver != null)
            {
                var message = _dpserver.GetString(header);
                WriteLine($"1. Client Connected with message: {message}");
            }
            else
            {
                WriteLine("1. Client Connected");
            }
        }

        void OnClientDisonnect(PacketHeader header)
        {
            if (header.DataSize > 0 && _dpserver != null)
            {
                string message = _dpserver.GetString(header);
                WriteLine($"21. Client Disconnected with message: {message}");
            }
            else
            {
                WriteLine("21. Client Disconnected");
            }
        }

        void OnInfoStringReceived(string? message)
        {
            if (message == "Info string from client")
			    WriteLine("2. Info string received");
        }

        void OnInfoDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(data);
                if (message == "Info data from client")
                    WriteLine("3. Info data received");
            }
        }

        void OnWarningStringReceived(string? message)
        {
            if (message == "Warning string from client")
                WriteLine("4. Warning string received");
        }

        void OnWarningDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(data);
                if (message == "Warning data from client")
                    WriteLine("5. Warning data received");
            }
        }

        void OnErrorStringReceived(string? message)
        {
            if (message == "Error string from client")
                WriteLine("6. Error string received");
        }

        void OnErrorDataReceived(byte[]? data)
        {
            if (data != null && data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(data);
                if (message == "Error data from client")
                    WriteLine("7. Error data received");

                messagesReceivedTrigger.SetComplete();
            }
        }

        void HandleCode1(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"14. Received request (handler 1): {message}");
                Thread.Sleep(3000);
                var response = request.CreateResponse();
                response.Code = 1;
                response.DataType = 2;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode2(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"15. Received request (handler 2): {message}");
                Thread.Sleep(2000);
                var response = request.CreateResponse();
                response.Code = 2;
                response.DataType = 3;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode3(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"16. Received request (handler 3): {message}");
                Thread.Sleep(1000);
                var response = request.CreateResponse();
                response.Code = 3;
                response.DataType = 4;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode4(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"17. Received request (handler 4): {message}");
                Thread.Sleep(3500);
                var response = request.CreateResponse();
                response.Code = 4;
                response.DataType = 5;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode5(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"18. Received request (handler 5): {message}");
                Thread.Sleep(2000);
                var response = request.CreateResponse();
                response.Code = 5;
                response.DataType = 6;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode6(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string message = _dpserver.Encoding.GetString(request.Data);
                WriteLine($"19. Received request (handler 6): {message}");
                Thread.Sleep(1000);
                var response = request.CreateResponse();
                response.Code = 6;
                response.DataType = 7;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode7(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string threadIdString = _dpserver.Encoding.GetString(request.Data);
                if (NewConsole)
                    WriteLine($"Received request (handler 7) sended sync from thread: {threadIdString}");
                var response = request.CreateResponse();
                response.Code = 7;
                response.DataType = 8;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode8(DPReceivedRequest request)
        {
            if (request.Data != null && request.Data.Length > 0 && _dpserver != null)
            {
                string threadIdString = _dpserver.Encoding.GetString(request.Data);
                if (NewConsole)
                    WriteLine($"Received request (handler 8) sended async from thread: {threadIdString}");
                var response = request.CreateResponse();
                response.Code = 8;
                response.DataType = 9;
                _dpserver.SendResponse(request, response);
            }
        }
        void HandleCode9(DPReceivedRequest request)
        {
            WriteLine("20. Received last request (handler 9)");
            var response = request.CreateResponse();
            response.Code = 9;
            response.DataType = 10;
            _dpserver?.SendResponse(request, response);
            lastRequestTrigger.SetComplete();
        }
        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipe = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipe == null)
                return;

            _dpserver = new DPServer(_dpipe, false, Encoding.Unicode);
            _dpserver.OnClientConnect = OnClientConnect;
            _dpserver.OnClientDisconnect = OnClientDisonnect;
            _dpserver.OnInfoStringReceived = OnInfoStringReceived;
            _dpserver.OnInfoDataReceived = OnInfoDataReceived;
            _dpserver.OnWarningStringReceived = OnWarningStringReceived;
            _dpserver.OnWarningDataReceived = OnWarningDataReceived;
            _dpserver.OnErrorStringReceived = OnErrorStringReceived;
            _dpserver.OnErrorDataReceived = OnErrorDataReceived;
            _dpserver.SetHandler(1, HandleCode1);
            _dpserver.SetHandler(2, HandleCode2);
            _dpserver.SetHandler(3, HandleCode3);
            _dpserver.SetHandler(4, HandleCode4);
            _dpserver.SetHandler(5, HandleCode5);
            _dpserver.SetHandler(6, HandleCode6);
            _dpserver.SetHandler(7, HandleCode7);
            _dpserver.SetHandler(8, HandleCode8);
            _dpserver.SetHandler(9, HandleCode9);

            //_dpipe.Start();
            _dpipe.StartWin32();
            var handle = _dpipe.GetHandle();
            bool processRun = RunClient(startParams, handle.AsString(), TestName);

            Trigger.Wait(messagesReceivedTrigger);

            WriteLine("8. Sending Info String");
            _dpserver.SendInfo("Info string from server");

            WriteLine("9. Sending Info Data");
            var infoData = _dpserver.Encoding.GetBytes("Info data from server");
            _dpserver.SendInfo(infoData);

            WriteLine("10. Sending Warning String");
            _dpserver.SendWarning("Warning string from server");

            WriteLine("11. Sending Warning Data");
            var warningData = _dpserver.Encoding.GetBytes("Warning data from server");
            _dpserver.SendWarning(warningData);

            WriteLine("12. Sending Error String");
            _dpserver.SendError("Error string from server");

            WriteLine("13. Sending Error Data");
            var errorData = _dpserver.Encoding.GetBytes("Error data from server");
            _dpserver.SendError(errorData);

            Trigger.Wait(lastRequestTrigger);

            Thread.Sleep(500);
        }
    }
}
