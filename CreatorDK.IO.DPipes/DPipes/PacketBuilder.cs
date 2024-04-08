using System.IO.Pipes;

namespace CreatorDK.IO.DPipes
{
    public class PacketBuilder(int bufferHeaderSize = (int)Constants.PACKET_HEADER_SIZE)
    {
        private readonly int _bufferHeaderSize = bufferHeaderSize;
        private readonly byte[] _pBufferHeaderIn = new byte[bufferHeaderSize];
        private readonly byte[] _pBufferHeaderOut = new byte[bufferHeaderSize];

        public PacketHeader GetPacketHeader(PipeStream pipeStream, DPipe dpipe) 
        {
            int nBytesRead = pipeStream.Read(_pBufferHeaderIn, 0, _bufferHeaderSize);

            if (nBytesRead < _bufferHeaderSize)
            {
                if (dpipe.IsAlive)
                    throw new Exception("Unable to read packet header from stream");
                else
                    return new PacketHeader(0, Constants.SERVICE_CODE_TERMINATING);
            }

            int dataSize = BitConverter.ToInt32(_pBufferHeaderIn, 0);
            uint serviceCode = BitConverter.ToUInt32(_pBufferHeaderIn, sizeof(uint));

            return new PacketHeader(dataSize, serviceCode);
        }

        public void PrepareHeader(uint serviceCodeRaw, int dataSize)
        {
            byte[] dataSizeBytes = BitConverter.GetBytes(dataSize);
            byte[] serviceCodeBytes = BitConverter.GetBytes(serviceCodeRaw);

            System.Buffer.BlockCopy(dataSizeBytes, 0, _pBufferHeaderOut, 0, dataSizeBytes.Length);
            System.Buffer.BlockCopy(serviceCodeBytes, 0, _pBufferHeaderOut, dataSizeBytes.Length, serviceCodeBytes.Length);
        }

        public void PrepareServiceHeader(uint serviceCode, int dataSize = 0)
        {
            uint serviceCodeRaw = serviceCode| 0x80000000;
            PrepareHeader(serviceCodeRaw, dataSize);
        }

        public void PrepareClientHeader(uint serviceCode, int dataSize)
        {
            uint serviceCodeRaw = serviceCode & 0x7FFFFFFF;
            PrepareHeader(serviceCodeRaw, dataSize);
        }

        public void PrepareClientHeader(int dataSize)
        {
            PrepareClientHeader(Constants.SERVICE_CODE_RAW_CLIENT, dataSize);
        }

        public void WriteHeader(Stream pipeStream)
        {
            pipeStream.Write(_pBufferHeaderOut, 0, _bufferHeaderSize);
        }

        public static uint GetCommand(uint serviceCode)
        {
            return serviceCode & 0x7FFFFFFF;
        }

        public static uint GetServiceCode(uint command)
        {
            return command | 0x80000000;
        }

        public byte[] GetBufferHeaderOut() 
        {
		    return _pBufferHeaderOut;
	    }

        public int BufferHeaderSize()
	    {
		    return _bufferHeaderSize;
	    }
    }
}
