using System.IO.Pipes;
using System.Text;

namespace CreatorDK.IO.DPipes
{
    public enum DPIPE_TYPE
    {
        ANONYMUS_PIPE = 1,
        NAMED_PIPE = 2
    }

    public enum DPIPE_MODE
    {
        UNSTARTED = 0,
        INNITIATOR = 1,
        CLIENT = 2,
        INNICIATOR_CLIENT
    }

    public abstract class DPipe
    {
        protected readonly string _name;
        protected DPIPE_MODE _mode = DPIPE_MODE.UNSTARTED;

        protected PipeStream? _readPipeStream;
        protected PipeStream? _writePipeStream;

        protected PacketBuilder _packetPuilder;

        protected int _bytesRead = 0;
        protected int _bytesToRead = 0;

        protected Thread? _readThread = null;
        protected Thread? _serviceThread = null;

        protected int _inBufferSize;
        protected int _outBufferSize;

        protected bool _isAlive = false;
        protected bool _listening = false;
        protected bool _otherSideDisconnecting = false;

        protected int _skipBufferSize = 4096;
        protected byte[] _skipBuffer;

        public Action<PacketHeader>? OnClientConnectCallback { get; set; }
        public Action<PacketHeader>? OnOtherSideDisconnectCallback { get; set; }
        public Action<PacketHeader>? OnPacketHeaderReceivedCallback { get; set; }

        public int InBufferSize => _inBufferSize;
        public int OutBufferSize => _outBufferSize;
        public abstract DPIPE_TYPE Type { get; }
        public DPIPE_MODE Mode => _mode;
        public string Name => _name;
        public int BytesToRead => _bytesToRead;
        public bool IsAlive => _isAlive;
        public PipeStream? ReadPipeStream => _readPipeStream;
        public PipeStream? WritePipeStream => _writePipeStream;
        protected DPipe(string name, int inBufferSize, int outBufferSize)
        {
            _name = name;
            _inBufferSize = inBufferSize;
            _outBufferSize = outBufferSize;
            _packetPuilder = new PacketBuilder();

            _skipBuffer = new byte[_skipBufferSize];
        }
        public virtual void ServicePacketReceived(PacketHeader ph)
        {
            uint command = PacketBuilder.GetCommand(ph.ServiceCode);

            switch (command)
            {
                case Constants.SERVICE_CODE_CONNECT:
                    OnClientConnect(ph);
                    break;
                case Constants.SERVICE_CODE_DISCONNECT:
                    OnOtherSideDisconnect(ph);
                    break;
                case Constants.SERVICE_CODE_TERMINATING:
                    OnTerminating();
                    break;
            }
        }
        protected virtual void OnClientConnect(PacketHeader ph)
        {
            _isAlive = true;

            OnClientConnectCallback?.Invoke(ph);
        }
        protected abstract void OnPipeClientConnect();
        protected virtual void OnOtherSideDisconnect(PacketHeader ph) 
        {
            _isAlive = false;

            OnOtherSideDisconnectCallback?.Invoke(ph);

            _otherSideDisconnecting = true;
        }
        protected virtual void OnOtherSideDisconnectPipe()
        {
            _isAlive = false;
            _listening = false;

            DisconnectPipe();

            _mode = DPIPE_MODE.UNSTARTED;
        }
        protected virtual void OnPacketHeaderReceived(PacketHeader ph)
        {
            OnPacketHeaderReceivedCallback?.Invoke(ph);
        }
        public virtual void OnTerminating() 
        {

        }
        public abstract int Read(byte[] buffer, int offset, int count);
        public virtual byte[]? Read(PacketHeader header, int offset = 0)
        {
            int bytesToRead = header.DataSize - offset;
            byte[]? data = null;

            if (bytesToRead > 0)
            {
                data = new byte[bytesToRead];
                Read(data, offset, bytesToRead);
            }

            return data;
        }
        public abstract void WritePacketHeader(PacketHeader header);
        public abstract void WriteRaw(byte[] buffer, int offset, int count);
        public abstract void Write(byte[] buffer, int offset, int count);
        public virtual void Write(byte[] buffer)
        {
            Write(buffer, 0, buffer.Length);
        }
        public abstract void Write(uint serviceCode, byte[] buffer, int offset, int count);
        public abstract IDPipeHandle GetHandle();
        public abstract string GetHandleString();
        public abstract void Start();
        public abstract void Connect(IDPipeHandle dPipeHandle);
        public abstract void Connect(IDPipeHandle dPipeHandle, byte[]? connectData);
        public virtual void Connect(IDPipeHandle dPipeHandle, string? connectMessage, Encoding? encoding = null)
        {
            encoding ??= Encoding.Default;

            byte[]? connectData = null;
            if (!string.IsNullOrEmpty(connectMessage))
                connectData = encoding.GetBytes(connectMessage);

            Connect(dPipeHandle, connectData);
        }
        public abstract void Connect(string handleString);
        public abstract void Connect(string handleString, byte[]? data);
        public virtual void Connect(string handleString, string? connectMessage, Encoding? encoding = null)
        {
            encoding ??= Encoding.Default;

            byte[]? connectData = null;

            if (connectMessage != null)
                connectData = encoding.GetBytes(connectMessage);

            Connect(handleString, connectData);
        }
        public virtual void Disconnect(byte[]? disconnectData)
        {
            if (_mode == DPIPE_MODE.UNSTARTED)
                return;

            if (!_otherSideDisconnecting && _isAlive)
            {
                int disconnectDataSize = disconnectData == null ? 0 : disconnectData.Length;

                _packetPuilder.PrepareServiceHeader(Constants.SERVICE_CODE_DISCONNECT, disconnectDataSize);

                if (_writePipeStream != null)
                    _packetPuilder.WriteHeader(_writePipeStream);

                if (disconnectData != null && disconnectDataSize > 0)
                {
                    WriteRaw(disconnectData, 0, disconnectDataSize);
                }
            }

            _readThread?.Join();
        }
        public virtual void Disconnect()
        {
            Disconnect(null);
        }
        public virtual void Disconnect(string? disconnectMessage, Encoding? encoding = null)
        {
            encoding ??= Encoding.Default;

            byte[]? disonnectData = null;

            if (!string.IsNullOrEmpty(disconnectMessage))
                disonnectData = encoding.GetBytes(disconnectMessage);

            Disconnect(disonnectData);
        }
        public abstract void DisconnectPipe();
        public virtual void Skip(int bytesToSkip) 
        {
            if (bytesToSkip > _bytesToRead)
                throw new Exception("Unable to skip more bytes then specified in PacketHeader");

            int cycles = bytesToSkip / _skipBufferSize;
            int restBytes = bytesToSkip % _skipBufferSize;

            for (int i = 0; i < cycles; i++)
            {
                Read(_skipBuffer, 0, _skipBufferSize);
            }

            if (restBytes > 0)
            {
                Read(_skipBuffer, 0, restBytes);
            }
        }
        public virtual void Skip(PacketHeader header)
        {
            Skip(header.DataSize);
        }
    }
}
