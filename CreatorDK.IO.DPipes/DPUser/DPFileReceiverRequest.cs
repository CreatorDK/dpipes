using System.IO;

namespace CreatorDK.IO.DPipes
{
    public class DPFileReceiverRequest
    {
        public string FileName { get; private set; }
        public byte[] FileData { get; private set; }
        public int FileSize { get; private set; }
        public bool FileDataAllocated { get; private set; }
        public IDPServer Server { get; private set; }

        public DPFileReceiverRequest(
            string fileName, 
            byte[] fileData, 
            int fileSize, 
            bool fileDataAllocated,
            IDPServer server)
        {
            FileName = fileName;
            FileData = fileData;
            FileSize = fileSize;
            FileDataAllocated = fileDataAllocated;
            Server = server;
        }

        public void SaveFile(FileStream fileStream, int bufferSize)
        {
            if (FileDataAllocated)
            {
                fileStream.Write(FileData, 0, FileData.Length);
            }
            else
            {
                var cycles = FileSize / bufferSize;
                var rest = FileSize % bufferSize;

                byte[] buffer = new byte[bufferSize];
                var pipe = Server.Pipe;

                for (int i = 0; i < cycles; i++)
                {
                    pipe.Read(buffer, 0, bufferSize);
                    fileStream.Write(buffer, 0, bufferSize);
                }

                pipe.Read(buffer, 0, rest);
                fileStream.Write(buffer, 0, rest);
            }
        }
    }
}
