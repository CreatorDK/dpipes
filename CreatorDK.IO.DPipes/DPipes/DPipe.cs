using System;
using System.IO;
using System.IO.Pipes;
using System.Text;
using System.Threading;

namespace CreatorDK.IO.DPipes
{
    public enum DP_TYPE
    {
        ANONYMOUS_PIPE = 1,
        NAMED_PIPE = 2
    }

    public enum DP_MODE
    {
        UNSTARTED = 0,
        INNITIATOR = 1,
        CLIENT = 2,
        INNICIATOR_CLIENT = 3
    }

    public abstract class DPipe
    {
        protected readonly string _name;
        protected DP_MODE _mode = DP_MODE.UNSTARTED;

        protected PipeStream _readPipeStream;
        protected PipeStream _writePipeStream;

        protected PacketBuilder _packetPuilder;

        protected int _bytesRead = 0;
        protected int _bytesToRead = 0;

        protected Thread _readThread = null;
        protected Thread _serviceThread = null;

        protected int _inBufferSize;
        protected int _outBufferSize;

        protected bool _isAlive = false;
        protected bool _listening = false;
        protected bool _otherSideDisconnecting = false;

        protected int _skipBufferSize = 4096;
        protected byte[] _skipBuffer;

        protected bool _clientEmulating = false;
        protected uint _mtu;

        public uint ConnectionTimeout { get; set; }

        public Action<PacketHeader> OnOtherSideConnectCallback { get; set; }
        public Action<PacketHeader> OnOtherSideDisconnectCallback { get; set; }
        public Action<PacketHeader> OnPacketHeaderReceivedCallback { get; set; }
        public Action<PacketHeader> OnPingReceivedCallback { get; set; }
        public Action<PacketHeader> OnPongReceivedCallback { get; set; }
        public Action<PacketHeader> OnConfigurationReceivedCallback { get; set; }

        public int InBufferSize => _inBufferSize;
        public int OutBufferSize => _outBufferSize;
        public abstract DP_TYPE Type { get; }
        public DP_MODE Mode => _mode;
        public string Name => _name;
        public int BytesToRead => _bytesToRead;
        public bool IsAlive => _isAlive;
        public PipeStream ReadPipeStream => _readPipeStream;
        public PipeStream WritePipeStream => _writePipeStream;
        protected DPipe(string name, uint mtu, int inBufferSize, int outBufferSize)
        {
            _mtu = mtu;
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
                case Constants.DP_SERVICE_CODE_CONNECT:
                    OnClientConnect(ph);
                    break;
                case Constants.DP_SERVICE_CODE_DISCONNECT:
                    OnOtherSideDisconnect(ph);
                    break;
                case Constants.DP_SERVICE_CODE_TERMINATING:
                    OnTerminating(ph);
                    break;
                case Constants.DP_SERVICE_CODE_PING:
                    OnPingReceivedInner(ph);
                    break;
                case Constants.DP_SERVICE_CODE_PONG:
                    OnPongReceivedInner(ph);
                    break;
                case Constants.DP_SERVICE_CODE_MTU_REQUEST:
                    OnMtuRequestReceived(ph);
                    break;
                case Constants.DP_SERVICE_CODE_MTU_RESPONSE:
                    OnMtuResponseReceived(ph);
                    break;
                case Constants.DP_SERVICE_CODE_SEND_CONFIGURATION:
                    OnConfigurationReceived(ph);
                    break;
            }
        }
        private void OnConfigurationReceived(PacketHeader ph)
        {
            if (OnConfigurationReceivedCallback != null)
            {
                OnConfigurationReceivedCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }

            else
                Skip(ph);
        }
        public void SendConfiguration(byte[] data)
        {
            Write(Constants.DP_SERVICE_CODE_SEND_CONFIGURATION, data, 0, data.Length);
        }
        public void SendPing(PipeStream stream)
        {
            PacketHeader header = new PacketHeader(true, 0);
            header.ServiceCode = Constants.DP_SERVICE_CODE_PING;
            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(stream);
        }
        public void SendPong(PipeStream stream)
        {
            PacketHeader header = new PacketHeader(true, 0);
            header.ServiceCode = Constants.DP_SERVICE_CODE_PONG;
            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(stream);
        }
        protected void OnPingReceivedInner(PacketHeader ph)
        {
            SendPong(_writePipeStream);

            if (OnPingReceivedCallback != null)
            {
                OnPingReceivedCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }

            else
                Skip(ph);
        }
        protected void OnPongReceivedInner(PacketHeader ph)
        {
            if (OnPongReceivedCallback != null)
            {
                OnPongReceivedCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }

            else
                Skip(ph);
        }
        protected virtual void OnMtuRequestReceived(PacketHeader ph)
        {
            byte[] mtuBytes = new byte[4];
            Read(mtuBytes, 0, 4);
            uint mtu = BitConverter.ToUInt32(mtuBytes, 0);

            if (mtu < _mtu)
                _mtu = mtu;

            SendMtuResponse(_writePipeStream, _mtu);
        }
        protected virtual void OnMtuResponseReceived(PacketHeader ph)
        {
            byte[] mtuBytes = new byte[4];
            Read(mtuBytes, 0, 4);
            uint mtu = BitConverter.ToUInt32(mtuBytes, 0);

            if (mtu < _mtu)
                _mtu = mtu;
        }
        protected virtual void SendMtuRequest(PipeStream pipe, uint mtu)
        {
            PacketHeader header = new PacketHeader(true, 4);
            header.ServiceCode = Constants.DP_SERVICE_CODE_MTU_REQUEST;
            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(_writePipeStream);
            byte[] mtuBytes = BitConverter.GetBytes(mtu);
            WriteRaw(pipe, mtuBytes, 0, 4);
        }
        protected virtual void SendMtuResponse(PipeStream pipe, uint mtu)
        {
            PacketHeader header = new PacketHeader(true, 4);
            header.ServiceCode = Constants.DP_SERVICE_CODE_MTU_RESPONSE;
            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(_writePipeStream);
            byte[] mtuBytes = BitConverter.GetBytes(mtu);
            WriteRaw(pipe, mtuBytes, 0, 4);
        }
        protected virtual void OnClientConnect(PacketHeader ph)
        {
            _isAlive = true;

            if (OnOtherSideConnectCallback != null)
            {
                OnOtherSideConnectCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }

            else
                Skip(ph);
        }
        protected abstract void OnPipeClientConnect();
        protected virtual void OnOtherSideDisconnect(PacketHeader ph) 
        {
            _isAlive = false;

             if (OnOtherSideDisconnectCallback != null)
            {
                OnOtherSideDisconnectCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }

             else
                Skip(ph);

            _otherSideDisconnecting = true;
        }
        protected virtual void OnOtherSideDisconnectPipe()
        {
            _isAlive = false;
            _listening = false;

            DisconnectPipe();

            _mode = DP_MODE.UNSTARTED;
        }
        protected virtual void OnPacketHeaderReceived(PacketHeader ph)
        {
            if (OnPacketHeaderReceivedCallback != null)
            {
                OnPacketHeaderReceivedCallback.Invoke(ph);

                if (_bytesToRead > 0)
                    Skip(_bytesToRead);
            }
                
            else
                Skip(ph);
        }
        public virtual void OnTerminating(PacketHeader ph) { }
        public abstract int Read(byte[] buffer, int offset, int count);
        public virtual byte[] Read(PacketHeader header, int offset = 0)
        {
            int bytesToRead = header.DataSize - offset;
            byte[] data = null;

            if (bytesToRead > 0)
            {
                data = new byte[bytesToRead];
                Read(data, (int)offset, (int)bytesToRead);
            }

            return data;
        }
        public abstract void WritePacketHeader(PacketHeader header);
        public abstract void WriteRaw(PipeStream pipeStream, byte[] buffer, int offset, int count);
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
        public abstract void Connect(IDPipeHandle dPipeHandle, byte[] connectData, uint prefix = 0);
        public abstract void Connect(string handleString);
        public abstract void Connect(string handleString, byte[] data, uint prefix = 0);
        public virtual void Disconnect(byte[] disconnectData, uint prefix = 0)
        {
            if (_mode == DP_MODE.UNSTARTED)
                return;

            if ((_mode == DP_MODE.INNITIATOR || _mode == DP_MODE.INNICIATOR_CLIENT) && !_isAlive)
            {
                OnPingReceivedCallback = null;
                OnPongReceivedCallback = null;
                OnOtherSideConnectCallback = null;
                OnOtherSideDisconnectCallback = null;
                OnPacketHeaderReceivedCallback = null;

                var handle = GetHandle();
                var virtualClient = CreateNewInstance();
                this._clientEmulating = true;
                virtualClient._clientEmulating = true;
                virtualClient.Connect(handle);
                virtualClient.Disconnect();

                while (virtualClient.Mode != DP_MODE.UNSTARTED)
                {
                    Thread.Sleep(1);
                }
                return;
            }

            if (!_otherSideDisconnecting && _isAlive)
            {
                int disconnectDataSize = disconnectData == null ? 0 : disconnectData.Length;

                PacketHeader header = new PacketHeader(true, disconnectDataSize);
                header.ServiceCode = Constants.DP_SERVICE_CODE_DISCONNECT;
                header.ServicePrefix = prefix;
                _packetPuilder.PrepareHeader(header);

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
        public abstract void DisconnectPipe();
        public virtual void Skip(int bytesToSkip) 
        {
            if (bytesToSkip == 0)
                return;

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
        public abstract DPipe CreateNewInstance();
        public virtual void WriteFrom(DPipe source, int count, int bufferLength)
        {
            byte[] buffer = new byte[bufferLength];

            var cyclecs = count / bufferLength;
            var rest = count % bufferLength;

            for (int i = 0; i < cyclecs; i++)
            {
                source.Read(buffer, 0, bufferLength);
                Write(buffer, 0, bufferLength);
            }

            source.Read(buffer, 0, rest);
            Write(buffer, 0, rest);
        }
        public virtual void WriteFrom(Stream source, int count, int bufferLength)
        {
            byte[] buffer = new byte[bufferLength];

            var cyclecs = count / bufferLength;
            var rest = count % bufferLength;

            for (int i = 0; i < cyclecs; i++)
            {
                source.Read(buffer, 0, bufferLength);
                Write(buffer, 0, bufferLength);
            }

            source.Read(buffer, 0, rest);
            Write(buffer, 0, rest);
        }
        public virtual void CopyTo(DPipe dest, int count, int bufferLength)
        {
            byte[] buffer = new byte[bufferLength];

            var cyclecs = count / bufferLength;
            var rest = count % bufferLength;

            for (int i = 0; i < cyclecs; i++)
            {
                Read(buffer, 0, bufferLength);
                dest.Write(buffer, 0, bufferLength);
            }

            Read(buffer, 0, rest);
            dest.Write(buffer, 0, rest);
        }
        public virtual void CopyTo(Stream dest, int count, int bufferLength)
        {
            byte[] buffer = new byte[bufferLength];

            var cyclecs = count / bufferLength;
            var rest = count % bufferLength;

            for (int i = 0; i < cyclecs; i++)
            {
                Read(buffer, 0, bufferLength);
                dest.Write(buffer, 0, bufferLength);
            }

            Read(buffer, 0, rest);
            dest.Write(buffer, 0, rest);
        }
    }
}
