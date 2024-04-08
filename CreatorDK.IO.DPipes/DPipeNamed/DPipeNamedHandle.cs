using System.Threading;
using System.Xml.Linq;

namespace CreatorDK.IO.DPipes
{
    public class DPipeNamedHandle : IDPipeHandle
    {
        public string ServerName { get; set; }
        public string PipeName { get; set; }

        public const string DefaultPipeName = "DPipeNamedDefault";
        public DPipeNamedHandle(string serverName, string pipeName)
        {
            ServerName = serverName;
            PipeName = pipeName;
        }
        public static DPipeNamedHandle Create(string pipeNameFull)
        {
            if (IsNamed(pipeNameFull))
            {
                var serverName = GetServerNamePart(pipeNameFull);
                var pipeName = GetNamedPipeNamePart(pipeNameFull);
                return new DPipeNamedHandle(serverName, pipeName);
            }
            else
                throw new ArgumentException("Invalid DPipeNamed pipe name");
        }
        public string AsString()
        {
            return $"\\\\{ServerName}\\pipe\\{PipeName}";
        }
        DPIPE_TYPE IDPipeHandle.GetType()
        {
            return DPIPE_TYPE.NAMED_PIPE;
        }
        public static bool IsNamed(string handleString)
        {
            if (handleString == null)
                return false;

            if (handleString.Length < 10)
                return false;

            if (handleString[0] != '\\' || handleString[1] != '\\')
                return false;

            int pipeWordIndex = handleString.IndexOf('\\', 2) + 1;

            string pipeWord = handleString.Substring(pipeWordIndex, 5);

            if (pipeWord == null || pipeWord != "pipe\\")
                return false;

            return true;
        }
        public static string GetServerNamePart(string pipeNameFull)
        {
            int indexOfServerName = pipeNameFull.IndexOf('\\', 2);
            return pipeNameFull.Substring(2, indexOfServerName - 2);
        }
        public static string GetNamedPipeNamePart(string pipeNameFull)
        {
            int indexOfPipeName = pipeNameFull.IndexOf("\\pipe\\", 2);
            return pipeNameFull.Substring(indexOfPipeName + 6);
        }
    }
}
