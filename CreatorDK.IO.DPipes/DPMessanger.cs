using System.Text;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public class DPMessanger : DPMessangerBase
    {
        private bool _handleAsync;

        public DPMessanger(DPipe dpipe, Encoding encoding, bool handleAsync = true, uint stringBufferSize = 4096) 
            : base(dpipe, encoding, stringBufferSize) 
        {
            _handleAsync = handleAsync;

            _dpipe.OnOtherSideConnectCallback = OnClientConnectInner;
            _dpipe.OnOtherSideDisconnectCallback = OnOtherSideDisconnectInner;
            _dpipe.OnPacketHeaderReceivedCallback = OnPacketHeaderReceivedInner;
        }

        ~DPMessanger()
        {
            _dpipe.OnOtherSideConnectCallback = null;
            _dpipe.OnOtherSideDisconnectCallback = null;
            _dpipe.OnPacketHeaderReceivedCallback = null;
        }

        public delegate void StringRecevivedHandler(string message);
        public StringRecevivedHandler OnClientConnect { get; set; }
        public StringRecevivedHandler OnOtherSideDisconect { get; set; }

        public void Connect(IDPipeHandle pipeHandle, string connectMessage = null, Encoding encoding = null)
        {
            if (string.IsNullOrEmpty(connectMessage))
                _dpipe.Connect(pipeHandle);
            else
            {
                var encodingCurrent = encoding == null ? Encoding : encoding;
                var data = encodingCurrent.GetBytes(connectMessage);
                var encodingCode = GetEncodingCode(encodingCurrent);
                _dpipe.Connect(pipeHandle, data, encodingCode);
            }
        }
        public void Connect(string pipeHandleString, string connectMessage = null, Encoding encoding = null)
        {
            if (string.IsNullOrEmpty(connectMessage))
                _dpipe.Connect(pipeHandleString);
            else
            {
                var encodingCurrent = encoding == null ? Encoding : encoding;
                var data = encodingCurrent.GetBytes(connectMessage);
                var encodingCode = GetEncodingCode(encodingCurrent);
                _dpipe.Connect(pipeHandleString, data, encodingCode);
            }
        }
        private void OnClientConnectInner(PacketHeader header)
        {
            var message = GetString(header);
            OnClientConnect?.Invoke(message);
        }
        private void OnOtherSideDisconnectInner(PacketHeader header)
        {
            var message = GetString(header);
            OnOtherSideDisconect?.Invoke(message);
        }
        private void OnPacketHeaderReceivedInner(PacketHeader header)
        {
            //Get data code ignoring first 8 bits (using for encoding)
            bool isStringData = (header.DataCodeOnly & 0x01) > 0;

            if (isStringData)
            {
                var message = GetString(header);

                if (_handleAsync)
                    Task.Run(() => OnMessageStringReceivedInner(header, message));
                else
                    OnMessageStringReceivedInner(header, message);
            }
            else if (header.DataCodeOnly > 0)
            {
                byte[] data = null;

                if (header.DataSize > 0)
                {
                    data = new byte[header.DataSize];
                    _dpipe.Read(data, 0, data.Length);
                }

                if (_handleAsync)
                    Task.Run(() => OnMessageDataReceivedInner(header, data));
                else
                    OnMessageDataReceivedInner(header, data);
            }
            else
            {
                var message = GetString(header);
                OnMessageStringReceived?.Invoke(message);
            }
        }
    }
}
