using CreatorDK.IO.DPipes;
using System.Diagnostics;

namespace DPipeTestCS.ClientTests
{
    [ClientTest("Test15", "Speed Test", "Description: Testing data transfering speed in raw DPipe class", true)]
    public class ClientTest15 : ClientTestBaseClass
    {
        public ClientTest15(string testName, string title, string description) : base(testName, title, description) { }

        DPipe? _dpipe;

        int _blockCount = 20;
        int _defualtBufferSize = 256 * 1024 * 1024;

        byte[] _buffer = Array.Empty<byte>();

        TimeSpan duractionTotal;
        Stopwatch stopWatch = new();

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

            if (startParams.Flags.ContainsKey("/s") && startParams.Flags["/s"].Length > 0)
                _buffer = new byte[Convert.ToInt32(startParams.Flags["/s"]) * 1024 * 1024];
            else
                _buffer = new byte[_defualtBufferSize];

            if (startParams.Flags.ContainsKey("/c") && startParams.Flags["/c"].Length > 0)
                _blockCount = Convert.ToInt32(startParams.Flags["/c"]);


            WriteLine("1. Connecting");
            _dpipe.Connect(startParams.PipeHandle);

            for (int i = 0; i < _blockCount; i++)
            {
                stopWatch.Start();

                _dpipe.Write(_buffer, 0, _buffer.Length);

                stopWatch.Stop();

                var duraction = stopWatch.Elapsed;

                stopWatch.Reset();

                duractionTotal += duraction;

                if (NewConsole)
                {
                    ClearCurrentConsoleLine();
                    Console.Write($"Current pipe write speed: {GetSpeed((ulong)_buffer.Length, duraction)}");
                }
            }

            if (NewConsole)
                Console.WriteLine();

            Thread.Sleep(500);

            ulong totalSize = (ulong)_blockCount * (ulong)_buffer.Length;

            WriteLine($"Total data size: {GetSize(totalSize)}");
            WriteLine($"Write time: {duractionTotal.ToString(@"mm\:ss\.fff")}");
            WriteLine($"Average Pipe Write Speed: {GetSpeed(totalSize, duractionTotal)}");
            WriteLine("2. Disconnecting");
            _dpipe.Disconnect();

            if (NewConsole)
                ReadKey();
        }
    }
}
