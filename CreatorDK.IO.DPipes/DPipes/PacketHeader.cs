using System;
using System.Xml.Linq;

namespace CreatorDK.IO.DPipes
{
    public class PacketHeader
    {
        private readonly int _dataSize;
        private uint _code;

        public PacketHeader(int dataSize, uint serviceCode)
        {
            _dataSize = dataSize;
            _code = serviceCode;
        }

        public PacketHeader(bool isService, int dataSize)
        {
            if (isService)
                _code = 2147483648;
            else
                _code = 0;

            _dataSize = dataSize;
        }

        public bool IsService => (_code & 0x80000000) > 0;
        public uint Code { get { return _code; } private set { _code = value; } }
        public int DataSize => _dataSize;

        public byte ServiceCode
        {
            get
            {
                if (!IsService)
                    throw new Exception("Unable to get service code from non-service packet");

                //Remove unused digits
                uint mask = 0xFF;
                uint codeLong = _code & mask;

                return (byte)codeLong;
            }

            set
            {
                if (!IsService)
                    throw new Exception("Unable to set service code on non-service packet");

                uint codeLong = value;

                _code = _code | codeLong;
            }
        }

        public uint ServicePrefix
        {
            get
            {
                if (!IsService)
                    throw new Exception("Unable to get service prefix from non-service packet");

                //Remove significant bit
                uint result = _code & 0x7FFFFFFF;
                return result >> 8;
            }

            set
            {
                if (!IsService)
                    throw new Exception("Unable to set service prefix from non-service packet");

                if (value > 16777215)
                    throw new Exception("Illegal prefix value. Prefix value range is 0..16777215");

                uint prefix = value << 23;
                _code = _code | prefix;
            }
        }

        public uint DataCode
        {
            get
            {
                if (IsService)
                    throw new Exception("Unable to get data code from service packet");

                //Remove significant bit
                return _code & 0x7FFFFFFF;
            }
            set
            {
                if (IsService)
                    throw new Exception("Unable to set data code on service packet");

                if (value > 2147483648)
                    throw new Exception("Illegal prefix value. Prefix value range is 0..2147483648");

                _code = _code | value;
            }
        }

        public uint DataCodeOnly
        {
            get
            {
                if (IsService)
                    throw new Exception("Unable to get data code from service packet");

                //Remove first 8 bits
                return _code & 0xFFFFFF;
            }
        }

        public byte DataPrefix
        {
            get
            {
                if (IsService)
                    throw new Exception("Unable to get data prefix from service packet");

                //Remove significant bit
                uint result = _code & 2147483647;

                result = result >> 24;

                return (byte)result;
            }
            set
            {
                if (IsService)
                    throw new Exception("Unable to set data prefix on service packet");

                if (value > 127)
                    throw new Exception("Illegal prefix value. Prefix value range is 0..127");

                uint prefixLong = value;
                prefixLong = prefixLong << 24;

                _code = _code | prefixLong;
            }
        }
    }
}
