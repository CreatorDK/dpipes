using Microsoft.Win32.SafeHandles;
using System;
using System.IO.Pipes;
using System.Security.Principal;
using System.Threading;

namespace CreatorDK.IO.DPipes
{
    public class DPNamed : DPipe
    {
        public DPNamed(string name = DPNamedHandle.DefaultPipeName, 
            int inBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT, 
            int outBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT) : 
                base(name, 65536, inBufferSize, outBufferSize) { }

        public DPNamed(
            int inBufferSize,
            int outBufferSize) :
                base(DPNamedHandle.DefaultPipeName, 65536, inBufferSize, outBufferSize)
        { }

        public static DPNamed Create(string pipeNameFull, int inBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT, int outBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT)
        {
            if (DPNamedHandle.IsNamed(pipeNameFull))
            {
                string pipeName = DPNamedHandle.GetNamedPipeNamePart(pipeNameFull);
                return new DPNamed(pipeName, inBufferSize, outBufferSize);
            }
                
            else
                throw new ArgumentException("Invalid named pipe name");
        }

        private TokenImpersonationLevel TokenImpersonationLevel { get; set; } = TokenImpersonationLevel.Impersonation;

        public override DP_TYPE Type => DP_TYPE.NAMED_PIPE;

        public int MaxInstances { get; set; } = 1;

        public int DefaultTimeout { get; set; }

        public bool UseRemote{  get; set; } = true;

        private void ReadLoop()
        {
            if (_readPipeStream == null || _writePipeStream == null)
                return;

            bool disconnection = false;

            if (_mode == DP_MODE.INNITIATOR)
            {
                var _readNamedPipeServerStream = (NamedPipeServerStream)_readPipeStream;
                var _writeNamedPipeServerStream = (NamedPipeServerStream)_writePipeStream;

                _readNamedPipeServerStream.WaitForConnection();
                _writeNamedPipeServerStream.WaitForConnection();
            }

            while (_listening)
            {
                if (_bytesToRead > 0)
                {
                    Thread.Sleep(1);
                    continue;
                }
                    
                PacketHeader header = _packetPuilder.GetPacketHeader(_readPipeStream, this);
                _bytesRead = 0;
                _bytesToRead = header.DataSize;

                if (header.IsService)
                {
                    var command = header.ServiceCode;

                    ServicePacketReceived(header);

                    if (command == Constants.DP_SERVICE_CODE_DISCONNECTED)
                    {
                        disconnection = true;
                        break;
                    }

                    else if (command == Constants.DP_SERVICE_CODE_DISCONNECT)
                    {
                        _packetPuilder.PrepareServiceHeader(Constants.DP_SERVICE_CODE_DISCONNECTED);
                        _packetPuilder.WriteHeader(_writePipeStream);
                        disconnection = true;
                        break;
                    }
                }
                else
                    OnPacketHeaderReceived(header);
            }

            if (disconnection)
                OnOtherSideDisconnectPipe();
        }
        public void Start(SafePipeHandle readPipeSafePipeHandle, SafePipeHandle writePipeSafePipeHandle) 
        {
            if (_mode == DP_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DP_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new NamedPipeServerStream(PipeDirection.In, false, false, readPipeSafePipeHandle);
            _writePipeStream = new NamedPipeServerStream(PipeDirection.Out, false, false, writePipeSafePipeHandle);

            _listening = true;
            _mode = DP_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public void Start(int maxNumberOfServerInstance, PipeOptions pipeOptions = PipeOptions.None)
        {
            if (_mode == DP_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DP_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            string pipeNameRead = $"{_name}_read";
            string pipeNameWrite = $"{_name}_write";

            _readPipeStream = new NamedPipeServerStream(
                pipeNameRead,
                PipeDirection.In,
                maxNumberOfServerInstance,
                PipeTransmissionMode.Byte,
                pipeOptions,
                _inBufferSize,
                _outBufferSize);

            _writePipeStream = new NamedPipeServerStream(
                pipeNameWrite,
                PipeDirection.Out,
                maxNumberOfServerInstance,
                PipeTransmissionMode.Byte,
                pipeOptions,
                _inBufferSize,
                _outBufferSize
                );

            _listening = true;
            _mode = DP_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override void Start()
        {
            Start(1, PipeOptions.None);
        }

        public void SetAccessControl(PipeSecurity pipeSecurity)
        {
            if (_mode == DP_MODE.CLIENT)
                throw new Exception("Cannot set Pipe Security properties in client mode");

            if (_mode == DP_MODE.UNSTARTED)
                throw new Exception("Cannot set Pipe Security properties in unstarted mode");

            if (_readPipeStream == null)
                return;

            NamedPipeServerStream _readStreamPipeServer = (NamedPipeServerStream)_readPipeStream;
            _readStreamPipeServer.SetAccessControl(pipeSecurity);
        }
        public override IDPipeHandle GetHandle()
        {
            if (_mode == DP_MODE.UNSTARTED)
                throw new Exception("Unable to get handle of unstarded dpipe");

            if (_mode == DP_MODE.CLIENT)
                throw new Exception("Unable to get handle in client mode");

            if (_mode == DP_MODE.INNICIATOR_CLIENT ||  _mode == DP_MODE.INNITIATOR)
            {
                if (UseRemote)
                    return new DPNamedHandle(Environment.MachineName, _name);
                else
                    return new DPNamedHandle(".", _name);
            }
               
            else
                throw new Exception("Undefined DPipe mode");
        }
        public override string GetHandleString()
        {
            return GetHandle().AsString();
        }
        public void ConnectNamed(DPNamedHandle pipeHandle, byte[] connectData = null, uint prefix = 0)
        {
            if (_mode == DP_MODE.CLIENT || _mode == DP_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is null");

            string serverName = pipeHandle.ServerName;

            string pipeClientReadHandle = pipeHandle.PipeName + "_write";
            string pipeClientWriteHandle = pipeHandle.PipeName + "_read";

            var readNamedPipeClientStream = 
                new NamedPipeClientStream(
                    serverName, 
                    pipeClientReadHandle, 
                    PipeDirection.In, 
                    PipeOptions.None, 
                    TokenImpersonationLevel);

            var writeNamedPipeClientStream =
                new NamedPipeClientStream(
                    serverName, 
                    pipeClientWriteHandle,
                    PipeDirection.Out,
                    PipeOptions.None,
                    TokenImpersonationLevel);

            _readPipeStream = readNamedPipeClientStream;
            _writePipeStream = writeNamedPipeClientStream;

            writeNamedPipeClientStream.Connect();
            readNamedPipeClientStream.Connect();

            SendMtuRequest(_writePipeStream, _mtu);

            var mtuHeader = _packetPuilder.GetPacketHeader(_readPipeStream, this);
            if (mtuHeader.ServiceCode != Constants.DP_SERVICE_CODE_MTU_RESPONSE)
                throw new Exception("Unable to get mtu size");

            _bytesToRead = 4;
            OnMtuResponseReceived(mtuHeader);

            _mode = DP_MODE.CLIENT;
            _listening = true;
            _isAlive = true;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();

            int connectDataSize = connectData == null ? 0 : connectData.Length;

            PacketHeader header = new PacketHeader(true, connectDataSize);
            header.ServiceCode = Constants.DP_SERVICE_CODE_CONNECT;
            header.ServicePrefix = prefix;

            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(_writePipeStream);

            if (connectData != null && connectDataSize > 0)
                WriteRaw(connectData, 0, connectDataSize);
        }
        public override void Connect(IDPipeHandle pipeHandle, byte[] connectData, uint prefix = 0)
        {
            if (_mode == DP_MODE.CLIENT || _mode == DP_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is null");

            DPNamedHandle namedHandle = (DPNamedHandle)pipeHandle;
            ConnectNamed(namedHandle, connectData, prefix);
        }
        public override void Connect(IDPipeHandle pipeHandle)
        {
            Connect(pipeHandle, null);
        }
        public override void Connect(string handleString, byte[] data, uint prefix = 0)
        {
            var namedHandle = DPNamedHandle.Create(handleString);
            ConnectNamed(namedHandle, data, prefix);
        }
        public override void Connect(string handleString)
        {
            var namedHandle = DPNamedHandle.Create(handleString);
            Connect(namedHandle, null);
        }
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (_readPipeStream == null)
                return 0;

            int bytesReadTotal = 0;

            int mtu = _mtu > int.MaxValue ? int.MaxValue : (int)_mtu;

            while (count != bytesReadTotal)
            {
                int bytesToRead;
                if (count - bytesReadTotal > mtu)
                    bytesToRead = mtu;
                else
                    bytesToRead = count - bytesReadTotal;

                int bytesRead = _readPipeStream.Read(buffer, offset + bytesReadTotal, bytesToRead);
                bytesReadTotal += bytesRead;
                _bytesRead += bytesRead;
                _bytesToRead -= bytesRead;
            }

            return bytesReadTotal;
        }

        public override void WriteRaw(PipeStream pipeStream, byte[] buffer, int offset, int count)
        {
            if (pipeStream == null)
                return;

            int bytesWrittenTotal = 0;

            int mtu = _mtu > int.MaxValue ? int.MaxValue : (int)_mtu;

            while (count != bytesWrittenTotal)
            {
                int bytesToWrite;
                if (count - bytesWrittenTotal > mtu)
                    bytesToWrite = mtu;
                else
                    bytesToWrite = count - bytesWrittenTotal;

                _writePipeStream.Write(buffer, offset + bytesWrittenTotal, bytesToWrite);
                bytesWrittenTotal += bytesToWrite;
            }
        }

        public override void WriteRaw(byte[] buffer, int offset, int count)
        {
            WriteRaw(_writePipeStream, buffer, offset, count);
        }
        public override void Write(uint serviceCode, byte[] buffer, int offset, int count)
        {
            if (_writePipeStream == null)
                return;

            _packetPuilder.PrepareClientHeader(serviceCode, count);
            _packetPuilder.WriteHeader(_writePipeStream);
            WriteRaw(buffer, offset, count);
        }
        public override void Write(byte[] buffer, int offset, int count)
        {
            Write(Constants.DP_SERVICE_CODE_RAW_CLIENT, buffer, offset, count);
        }
        public override void WritePacketHeader(PacketHeader header)
        {
            if (_writePipeStream == null)
                return;

            _packetPuilder.PrepareClientHeader(header.Code, header.DataSize);
            _packetPuilder.WriteHeader(_writePipeStream);
        }
        protected override void OnPipeClientConnect()
        {
            if (_readPipeStream == null || _writePipeStream == null)
                return;

            _readPipeStream?.Dispose();
            _writePipeStream?.Dispose();
        }
        public override void DisconnectPipe()
        {
            _readPipeStream?.Dispose();
            _writePipeStream?.Dispose();
        }

        public override DPipe CreateNewInstance()
        {
            return new DPNamed(_name, _inBufferSize, _outBufferSize);
        }
    }
}
