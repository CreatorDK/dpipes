using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using CreatorDK.Triggers;
using System.Diagnostics;

namespace DPipeTestCS.ServerTests
{
    [ServerTest("Test15", "Speed Test", "Description: Testing data transfering speed in raw DPipe class", true)]
    public class ServerTest15 : ServerTestBaseClass
    {
        public ServerTest15(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;

        int _blockCount = 20;
        int _defualtBufferSize = 256 * 1024 * 1024;

        byte[] _buffer = Array.Empty<byte>();

        BoolTrigger testEndTrigger = new();

        int _blockReceivedCount = 0;

        TimeSpan duractionTotal;
        Stopwatch stopWatch = new();

        void OnClientConnect(PacketHeader header)
        {
            WriteLine("1. Client Connected");
        }

        void OnOtherSideDisconnect(PacketHeader header)
        {
            WriteLine("2. Client Disconecting");
        }

        string GetSpeed(ulong bytes, TimeSpan duraction)
        {
            double bytesF = bytes;
            double duractionSecondsF = (double)duraction.TotalNanoseconds / 1000000000d;
            double resultF = bytesF / duractionSecondsF;

            if (resultF > 1099511627776d)
                return $"{(resultF / 1099511627776d).ToString("0.000")} Tb/s";

            else if (resultF > 1073741824d)
                return $"{(resultF / 1073741824d).ToString("0.000")} Gb/s";

            else if (resultF > 1048576d)
                return $"{(resultF / 1048576d).ToString("0.000")} Mb/s";

            else if (resultF > 1024d)
                return $"{(resultF / 1024d).ToString("0.000")} Kb/s";
            else
                return $"{resultF.ToString("0.000")} b/s";
        }

        string GetSize(ulong bytes)
        {
            double bytesF = bytes;

            if (bytesF > 1099511627776d)
                return $"{(bytesF / 1099511627776d).ToString("0.000")} Tb";

            else if (bytesF > 1073741824d)
                return $"{(bytesF / 1073741824d).ToString("0.000")} Gb";

            else if (bytesF > 1048576d)
                return $"{(bytesF / 1048576d).ToString("0.000")} Mb";

            else if (bytesF > 1024d)
                return $"{(bytesF / 1024d).ToString("0.000")} Kb";

            else
                return $"{bytesF.ToString("0.000")} b";
        }

        public static void ClearCurrentConsoleLine()
        {
            int currentLineCursor = Console.CursorTop;
            Console.SetCursorPosition(0, Console.CursorTop);
            Console.Write(new string(' ', Console.WindowWidth));
            Console.SetCursorPosition(0, currentLineCursor);
        }

        void OnPacketHeaderReceived(PacketHeader header)
        {
            if (header.DataSize != _buffer.Length)
                throw new Exception("Wrong data size");

            stopWatch.Start();

            _dpipe?.Read(_buffer, 0, header.DataSize);

            stopWatch.Stop();

            var duraction = stopWatch.Elapsed;

            duractionTotal += duraction;

            stopWatch.Reset();

            if (NewConsole)
            {
                ClearCurrentConsoleLine();
                Console.Write($"Current pipe read speed: {GetSpeed((ulong)header.DataSize, duraction)}");
            }

            ++_blockReceivedCount;

            if (_blockReceivedCount == _blockCount)
            {
                if (NewConsole)
                    Console.WriteLine();

                ulong totalSize = (ulong)_blockCount * (ulong)_buffer.Length;

                WriteLine($"Total data size: {GetSize(totalSize)}");
                WriteLine($"Reading time: {duractionTotal.ToString(@"mm\:ss\.fff")}");
                WriteLine($"Average Pipe Read Speed: {GetSpeed(totalSize, duractionTotal)}");

                testEndTrigger.SetComplete();
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
            _dpipe.OnOtherSideDisconnectCallback = OnOtherSideDisconnect;

            if (startParams.Flags.ContainsKey("/s") && startParams.Flags["/s"].Length > 0)
                _buffer = new byte[Convert.ToInt32(startParams.Flags["/s"]) * 1024 * 1024];
            else
                _buffer = new byte[_defualtBufferSize];

            if (startParams.Flags.ContainsKey("/c") && startParams.Flags["/c"].Length > 0)
                _blockCount = Convert.ToInt32(startParams.Flags["/c"]);

            //_dpipe.Start();
            _dpipe.StartWin32();
            var handle = _dpipe.GetHandle();
            bool processRun = RunClient(startParams, handle.AsString(), TestName);

            Trigger.Wait(testEndTrigger);

            Thread.Sleep(500);
        }
    }
}
