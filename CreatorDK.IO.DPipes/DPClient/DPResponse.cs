using System;

namespace CreatorDK.IO.DPipes
{
    public class DPResponse
    {
        private int _code;
        private int _descriptorType;
        private int _descriptorSize;
        private bool _descriptorAllocated;
        private byte[] _descriptor;
        private int _dataType;
        private int _dataSize;
        private bool _dataAllocated;
        private byte[] _data;
        DPipe _dpipe;
        public int Code => _code;
        public int DescriptorType => _descriptorType;
        public int DescriptorSize => _descriptorSize;
        public bool DescriptorAllocated => _descriptorAllocated;
        public byte[] Descriptor => _descriptor;
        public int DataType => _dataType;
        public int DataSize => _dataSize;
        public bool DataAllocated => _dataAllocated;
        public byte[] Data => _data;

        private TimeSpan _send_duraction = new TimeSpan();
        private TimeSpan _request_duraction = new TimeSpan();

        public DPResponse(
            int code, 
            int descriptorType, 
            int descriptorSize, 
            bool descriptorAllocated,
            byte[] descriptor,
            int dataType, 
            int dataSize, 
            bool dataAllocated, 
            byte[] data,
            DPipe dpipe
            )
        {
            _code = code;
            _descriptorType = descriptorType;
            _descriptorSize = descriptorSize;
            _descriptorAllocated = descriptorAllocated;
            _descriptor = descriptor;
            _dataType = dataType;
            _dataSize = dataSize;
            _dataAllocated = dataAllocated;
            _data = data;
            _dpipe = dpipe;
        }

        public void ReadDescriptor()
        {
            if (!_descriptorAllocated && _descriptorSize > 0)
            {
                _descriptor = new byte[_descriptorSize];
                _dpipe.Read(_descriptor, 0, _descriptorSize);
                _descriptorAllocated = true;
            }
        }

        public void Read() 
        {
            ReadDescriptor();

            if (!_dataAllocated && _descriptorSize > 0 ) 
            { 
                _data = new byte[_dataSize];
                _dpipe.Read(_data, 0, _dataSize);
                _dataAllocated = true;
            }
        }
        
        public void SetSendDuraction(TimeSpan send_duraction)
        {
            _send_duraction = send_duraction;
        }

        public void SetRequestDuraction(TimeSpan request_duraction)
        {
            _request_duraction = request_duraction;
        }

        public TimeSpan SendDuraction => _send_duraction;
        public TimeSpan RequestDuraction => _request_duraction;
        public TimeSpan TotalDuraction => _send_duraction + _request_duraction;
    }
}
