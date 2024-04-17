using System;

namespace CreatorDK.IO.DPipes
{
    public class DPReceivedRequest
    {
        private Guid _guid;
        private int _code;
        private int _descriptorType;
        private int _descriptorSize;
        private bool _descriptorAllocated;
        private byte[] _descriptor;
        private int _dataType;
        private int _dataSize;
        private bool _dataAllocated;
        private byte[] _data;
        private IDPServer _server;
        public Guid Guid => _guid;
        public int Code => _code;
        public int DescriptorType => _descriptorType;
        public int DescriptorSize => _descriptorSize;
        public bool DescriptorAllocated => _descriptorAllocated;
        public byte[] Descriptor => _descriptor;
        public int DataType => _dataType;
        public int DataSize => _dataSize;
        public bool DataAllocated => _dataAllocated;
        public byte[] Data => _data;

        public IDPServer Server => _server;
        public DPReceivedRequest(
            Guid guid,
            int code,
            int descriptorType,
            int descriptorSize,
            bool descriptorAllocated,
            byte[] descriptor,
            int dataType,
            int dataSize,
            bool dataAllocated,
            byte[] data,
            IDPServer server)
        {
            _guid = guid;
            _code = code;
            _descriptorType = descriptorType;
            _descriptorSize = descriptorSize;
            _descriptorAllocated = descriptorAllocated;
            _descriptor = descriptor;
            _dataType = dataType;
            _dataSize = dataSize;
            _dataAllocated = dataAllocated;
            _data = data;
            _server = server;
        }

        public void ReadDescriptor()
        {
            if (!_descriptorAllocated && _descriptorSize > 0)
            {
                _descriptor = new byte[_descriptorSize];
                _server.Pipe.Read(_descriptor, 0, _descriptorSize);
                _descriptorAllocated = true;
            }
        }

        public void Read()
        {
            ReadDescriptor();

            if (!_dataAllocated && _descriptorSize > 0)
            {
                _data = new byte[_dataSize];
                _server.Pipe.Read(_data, 0, _dataSize);
                _dataAllocated = true;
            }
        }

        public DPClientResponse CreateResponse()
        {
            return new DPClientResponse { Guid = _guid };
        }
    }
}
