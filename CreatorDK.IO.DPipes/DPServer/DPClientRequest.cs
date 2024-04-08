using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public class DPClientRequest
    {
        public DPClientRequest(Guid guid, int code, int dataType, byte[]? data)
        {
            _guid = guid;
            _code = code;
            _dataType = dataType;
            _data = data;
        }

        private Guid _guid;
        private int _code;
        private int _dataType;
        private byte[]? _data;

        public Guid Guid => _guid;
        public int Code => _code;
        public int DataType => _dataType;
        public byte[]? Data => _data;

        public DPClientResponse CreateResponse()
        {
            return new DPClientResponse { Guid = _guid };
        }
    }
}
