using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using Microsoft.VisualStudio.OLE.Interop;
using System;
using System.Drawing;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("FileTransferTest", "File Transfer Test", "Description: Testing transfering file throw the dpipe", false)]
    public class ServerTestFileTransfer : ServerTestBaseClass
    {
        public ServerTestFileTransfer(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipeServer;
        DPipe? _dpipeClient;

        string fileName = string.Empty;
        BoolTrigger testEndTrigger = new BoolTrigger();

        void OnClientConnect(PacketHeader header)
        {
            Write("1. Client Connected");
        }

        private void OnClientDisconnect(PacketHeader header)
        {
            Write("2. Client Disconnected");
        }

        private void OnPacketHeaderReceivedCallback(PacketHeader header)
        {
            int dataSize = header.DataSize;

            string extension = Path.GetExtension(fileName);
            string newFileName = fileName.Replace(extension, "") + "_copy" + extension; ;

            var file = File.Open(newFileName, FileMode.CreateNew);

            byte[] buffer = new byte[10240];

            var cycles = header.DataSize / 10240;
            var rest = header.DataSize % 10240;

            for (int i = 0; i < cycles; i++)
            {
                _dpipeServer?.Read(buffer, 0, 10240);
                file.Write(buffer, 0, 10240);
            }

            _dpipeServer?.Read(buffer, 0, rest);
            file.Write(buffer, 0, rest);
            file.Close();

            testEndTrigger.SetComplete();
        }

        void SendDataFile(string fileName)
        {
            var file = File.Open(fileName, FileMode.Open);
            int size = (int)file.Length;
            Console.WriteLine($"File size is  {size}");

            byte[] buffer = new byte[size];

            file.Read(buffer, 0, size);
            _dpipeClient?.Write(buffer, 0, size);
            file.Close();
        }

        public override void Execute(StartParamsServer startParams)
        {
            WriteTestName(startParams.PIPE_TYPE);
            _dpipeServer = DPipeBuilder.Create(startParams.PIPE_TYPE);

            if (_dpipeServer == null)
                return;

            _dpipeServer.OnOtherSideConnectCallback = OnClientConnect;
            _dpipeServer.OnOtherSideDisconnectCallback = OnClientDisconnect;
            _dpipeServer.OnPacketHeaderReceivedCallback = OnPacketHeaderReceivedCallback;
            _dpipeServer.StartWin32();

            var handleString = _dpipeServer.GetHandleString();
            _dpipeClient = DPipeBuilder.Create(handleString);
            _dpipeClient.Connect(handleString);

            foreach (var flag in startParams.Flags)
            {
                if (flag.Key == "/file" && flag.Value.Length > 0)
				fileName = flag.Value;
            }

		    if (fileName.Length <= 0) {
			    Console.WriteLine("Unable to get file name");
			    return;
		    }

            SendDataFile(fileName);
            Trigger.Wait(testEndTrigger);
        }
    }
}
