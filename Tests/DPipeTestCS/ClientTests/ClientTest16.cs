using CreatorDK.IO.DPipes;
using System.Diagnostics;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test16", "Test Skip Method", "Description: Testing Skip method when run manualy and in case when OnDisconnect function is not set", true)]
    public class ClientTest16 : ClientTestBaseClass
    {
        public ClientTest16(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;
        byte[] _buffer = new byte[186442];

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

            Random rnd = new Random();

            for (int i = 0; i < _buffer.Length; i++)
            {
                _buffer[i] = (byte)rnd.Next(255);
            }


            WriteLine("1. Connecting and send data");
            _dpipe.Connect(startParams.PipeHandle, _buffer);

            Thread.Sleep(100);

            WriteLine("2. Send data");
            _dpipe.Write(_buffer);

            Thread.Sleep(100);

            WriteLine("2. Disconnecting");
            _dpipe.Disconnect(_buffer);

            if (NewConsole)
                ReadKey();
        }
    }
}
