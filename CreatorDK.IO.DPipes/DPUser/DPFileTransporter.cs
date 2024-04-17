using System;
using System.IO;
using System.Text;

namespace CreatorDK.IO.DPipes
{
    public delegate void DPReceivedRequestDelegate(DPFileReceiverRequest request);

    public class DPFileTransporter
    {
        private readonly DPUser _dpuser;
        private readonly int _code;

        public DPReceivedRequestDelegate OnFileReceived;

        public DPFileTransporter(DPUser dpuser, int code)
        {
            _dpuser = dpuser;
            _code = code;

            _dpuser.SetHandler(code, OnFileRequestReceived);
        }
        ~DPFileTransporter()
        {
            _dpuser.RemoveHandler(_code);
        }

        private void OnFileRequestReceived(DPReceivedRequest request)
        {
            if (OnFileReceived != null)
            {
                var encodingCode = request.DescriptorType;

                if (!request.DescriptorAllocated)
                {
                    request.ReadDescriptor();
                }

                var encoding = DPMessangerBase.GetEncoding((uint)encodingCode);
                var fileName = encoding.GetString(request.Descriptor);

                DPFileReceiverRequest requestToSend = new DPFileReceiverRequest(fileName, request.Data, request.DataSize, request.DataAllocated, request.Server);

                OnFileReceived.Invoke(requestToSend);
            }
        }

        public void SendFile(FileStream fileStream, string fileName, int bufferSize)
        {
            var fileSize = fileStream.Length;

            var descriptor = Encoding.Unicode.GetBytes(fileName);

            DPRequest req = new DPRequest();
            req.Code = _code;
            req.DataType = 0;
            req.DescriptorType = (int)DPMessangerBase.GetEncodingCode(Encoding.Unicode);
            req.DescriptorSize = descriptor.Length;

            DPRequestRecord requestRecord = new DPRequestRecord()
            {
                Guid = Guid.NewGuid()
            };

            _dpuser.PrepareRequestRecord(req, requestRecord);

            var packetSize = (int)(Constants.DP_REQUEST_SIZE + descriptor.Length + fileSize);

            PacketHeader header = new PacketHeader(false, packetSize);
            header.DataCode = Constants.DP_REQUEST;

            var writeMutex = _dpuser.WriteMutex;
            var pipe = _dpuser.Pipe;

            writeMutex.WaitOne();

            {
                pipe.WritePacketHeader(header);
                pipe.WriteRaw(_dpuser.RequestBuffer, 0, (int)Constants.DP_REQUEST_SIZE);

                if (descriptor.Length > 0)
                    pipe.WriteRaw(descriptor, 0, descriptor.Length);

                var buffer = new byte[bufferSize];

                var cycles = fileSize / bufferSize;
                var rest = (int)(fileSize % bufferSize);

                for (int i = 0; i < cycles; i++)
                {
                    fileStream.Read(buffer, 0, bufferSize);
                    pipe.WriteRaw(buffer, 0, bufferSize);
                }

                fileStream.Read(buffer, 0, rest);
                pipe.Write(buffer, 0, rest);
            }

            writeMutex.ReleaseMutex();
        } 
    }
}
