using System;
using System.Globalization;
using System.Text;
using System.Text.RegularExpressions;

namespace CreatorDK.IO.DPipes
{
    public partial class DPAnonymousHandle : IDPipeHandle
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

        public DPAnonymousHandle(int readHandle, int writeHandle)
        {
            _readHandle = readHandle.ToString();
            _writeHandle = writeHandle.ToString();
        }

        private DPAnonymousHandle(string handleString)
        {
            string readHandleString = handleString.Substring(0, 16);
            string writeHandleString = handleString.Substring(19, 16);

            int readHandleInteger = int.Parse(readHandleString, NumberStyles.HexNumber);
            int writeHandleInteger = int.Parse(writeHandleString, NumberStyles.HexNumber);

            _readHandle = readHandleInteger.ToString();
            _writeHandle = writeHandleInteger.ToString();
        }

        public static DPAnonymousHandle Create(string handleString)
        {
            if (IsAnonymus(handleString))
                return new DPAnonymousHandle(handleString);
            else
                throw new ArgumentException("Invalid DPipeAnonymus handle string");
        }

        public DPAnonymousHandle(string readHandleString, string writeHandleString)
        {
            _readHandle = readHandleString;
            _writeHandle = writeHandleString;
        }

        public static DPAnonymousHandle Create(string readHandleString, string writeHandleString)
        {
            bool readHandleCorrect = int.TryParse(readHandleString, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _);

            if (!readHandleCorrect)
                throw new ArgumentException("Invalid DPipeAnonymus Read Handle");

            bool writeHandleCorrect = int.TryParse(writeHandleString, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out _);

            if (!writeHandleCorrect)
                throw new ArgumentException("Invalid DPipeAnonymus Write Handle");

            return new DPAnonymousHandle(readHandleString, writeHandleString);
        }

        public string AsString()
        {
            int readHandleInteger = int.Parse(_readHandle);
            int writeHandleInteger = int.Parse(_writeHandle);

            string readHandleString = readHandleInteger.ToString("X16");
            string writeHandleString = writeHandleInteger.ToString("X16");
            return $"{readHandleString}:::{writeHandleString}";
        }

        DP_TYPE IDPipeHandle.GetType()
        {
            return DP_TYPE.ANONYMOUS_PIPE;
        }

        public static Regex AnonymusHandleRegex()
        {
            return new Regex("[0-9A-F]{16}:::[0-9A-F]{16}");
        }
    }
}
