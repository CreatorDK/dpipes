namespace CreatorDK.IO.DPipes
{
    public class PacketHeader(int dataSize, uint serviceCode)
    {
        private readonly uint _serviceCode = serviceCode;
        private readonly int _dataSize = dataSize;

        public int DataSize => _dataSize;
        public uint ServiceCode => _serviceCode;

        public uint Command => _serviceCode & 0x7FFFFFFF;

        public bool IsService()
        {
            return (_serviceCode & 0x80000000) > 0;
        }
    }
}
