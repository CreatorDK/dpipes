using System.Text;

namespace CreatorDK.IO.DPipes
{
    public class DPipeMessanger : DPipeMessangerBase
    {
        private bool _handleAsync;

        public DPipeMessanger(DPipe dpipe, Encoding encoding, bool handleAsync = true, uint stringBufferSize = 4096) 
            : base(dpipe, encoding, stringBufferSize) 
        {
            _handleAsync = handleAsync;

            _dpipe.OnClientConnectCallback = OnClientConnectInner;
            _dpipe.OnOtherSideDisconnectCallback = OnOtherSideDisconnectInner;
            _dpipe.OnPacketHeaderReceivedCallback = OnPacketHeaderReceivedInner;
        }

        ~DPipeMessanger()
        {
            _dpipe.OnClientConnectCallback = null;
            _dpipe.OnOtherSideDisconnectCallback = null;
            _dpipe.OnPacketHeaderReceivedCallback = null;
        }

        public delegate void StringRecevivedHandler(string? message);
        public StringRecevivedHandler? OnClientConnect { get; set; }
        public StringRecevivedHandler? OnOtherSideDisconect { get; set; }

        public void Connect(IDPipeHandle pipeHandle, string? connectMessage = null)
        {
            _dpipe.Connect(pipeHandle, connectMessage, _encoding);
        }

        public void Connect(string pipeHandleString, string? connectMessage = null)
        {
            _dpipe.Connect(pipeHandleString, connectMessage, _encoding);
        }

        public void Disconnect(string? disconnectMessage = null)
        {
            _dpipe.Disconnect(disconnectMessage, _encoding);
        }

        private void OnClientConnectInner(PacketHeader header)
        {
            var message = GetStringFromPipe(header.DataSize);
            OnClientConnect?.Invoke(message);
        }
        private void OnOtherSideDisconnectInner(PacketHeader header)
        {
            var message = GetStringFromPipe(header.DataSize);
            OnOtherSideDisconect?.Invoke(message);
        }
        private void OnPacketHeaderReceivedInner(PacketHeader header)
        {
            bool isStringData = (header.Command & 0x01) > 0;

            if (isStringData)
            {
                var message = GetStringFromPipe(header.DataSize);

                if (_handleAsync)
                    Task.Run(() => OnMessageStringReceivedInner(header, message));
                else
                    OnMessageStringReceivedInner(header, message);
            }
            else if (header.Command > 0)
            {
                byte[]? data = null;

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
                var message = GetStringFromPipe(header.DataSize);
                OnMessageStringReceived?.Invoke(message);
            }
        }
    }
}
