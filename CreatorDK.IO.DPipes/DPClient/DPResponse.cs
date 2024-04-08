namespace CreatorDK.IO.DPipes
{
    public class DPResponse
    {
        private int _code;
        private int _dataType;
        private byte[]? _data;
        public int Code => _code;
        public int DataType => _dataType;
        public byte[]? Data => _data;

        private TimeSpan _send_duraction = new();
        private TimeSpan _request_duraction = new();

        public DPResponse(int code, int dataType, byte[]? data)
        {
            _code = code;
            _dataType = dataType;
            _data = data;
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
