using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace CreatorDK.IO.DPipes
{
    public partial class DPipeAnonymousHandle : IDPipeHandle
    {
        private readonly string _readHandle;
        private readonly string _writeHandle;

        public string ReadHandle => _readHandle;
        public string WriteHandle => _writeHandle;

        public static bool IsAnonymus(string handleString)
        {
            return AnonymusHandleRegex().Match(handleString).Success;
        }

        public const string DefaultPipeName = "Anonymus";

        public DPipeAnonymousHandle(int readHandle, int writeHandle)
        {
            _readHandle = readHandle.ToString();
            _writeHandle = writeHandle.ToString();
        }

        private DPipeAnonymousHandle(string handleString)
        {
            string readHandleString = handleString[..16];
            string writeHandleString = handleString.Substring(19, 16);

            int readHandleInteger = int.Parse(readHandleString, NumberStyles.HexNumber);
            int writeHandleInteger = int.Parse(writeHandleString, NumberStyles.HexNumber);

            _readHandle = readHandleInteger.ToString();
            _writeHandle = writeHandleInteger.ToString();
        }

        public static DPipeAnonymousHandle Create(string handleString)
        {
            if (IsAnonymus(handleString))
                return new DPipeAnonymousHandle(handleString);
            else
                throw new ArgumentException("Invalid DPipeAnonymus handle string");
        }

        public DPipeAnonymousHandle(string readHandleString, string writeHandleString)
        {
            _readHandle = readHandleString;
            _writeHandle = writeHandleString;
        }

        public static DPipeAnonymousHandle Create(string readHandleString, string writeHandleString)
        {
            bool readHandleCorrect = int.TryParse(Encoding.UTF8.GetBytes(readHandleString), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _);

            if (!readHandleCorrect)
                throw new ArgumentException("Invalid DPipeAnonymus Read Handle");

            bool writeHandleCorrect = int.TryParse(Encoding.UTF8.GetBytes(writeHandleString), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _);

            if (!writeHandleCorrect)
                throw new ArgumentException("Invalid DPipeAnonymus Write Handle");

            return new DPipeAnonymousHandle(readHandleString, writeHandleString);
        }

        public string AsString()
        {
            int readHandleInteger = int.Parse(_readHandle);
            int writeHandleInteger = int.Parse(_writeHandle);

            string readHandleString = readHandleInteger.ToString("X16");
            string writeHandleString = writeHandleInteger.ToString("X16");
            return $"{readHandleString}:::{writeHandleString}";
        }

        DPIPE_TYPE IDPipeHandle.GetType()
        {
            return DPIPE_TYPE.ANONYMUS_PIPE;
        }

        [GeneratedRegex("[0-9A-F]{16}:::[0-9A-F]{16}")]
        private static partial Regex AnonymusHandleRegex();
    }
}
