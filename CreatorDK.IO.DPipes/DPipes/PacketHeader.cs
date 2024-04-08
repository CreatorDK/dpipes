namespace CreatorDK.IO.DPipes
{
    public class PacketHeader
    {
        private readonly int _dataSize;
        private readonly uint _serviceCode;

        public PacketHeader(int dataSize, uint serviceCode)
        {
            _dataSize = dataSize;
            _serviceCode = serviceCode;
        }

        public int DataSize => _dataSize;
        public uint ServiceCode => _serviceCode;

        public uint Command => _serviceCode & 0x7FFFFFFF;

        public bool IsService()
        {
            return (_serviceCode & 0x80000000) > 0;
        }
    }
}
