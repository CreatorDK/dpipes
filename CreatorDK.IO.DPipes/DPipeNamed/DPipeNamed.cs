using Microsoft.Win32.SafeHandles;
using System;
using System.IO.Pipes;
using System.Security.Principal;
using System.Threading;

namespace CreatorDK.IO.DPipes
{
    public class DPipeNamed : DPipe
    {
        public DPipeNamed(string name = DPipeNamedHandle.DefaultPipeName, 
            int inBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT, 
            int outBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT) : 
                base(name, inBufferSize, outBufferSize) { }

        public DPipeNamed(
            int inBufferSize,
            int outBufferSize) :
                base(DPipeNamedHandle.DefaultPipeName, inBufferSize, outBufferSize)
        { }

        public static DPipeNamed Create(string pipeNameFull, int inBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT, int outBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT)
        {
            if (DPipeNamedHandle.IsNamed(pipeNameFull))
            {
                string pipeName = DPipeNamedHandle.GetNamedPipeNamePart(pipeNameFull);
                return new DPipeNamed(pipeName, inBufferSize, outBufferSize);
            }
                
            else
                throw new ArgumentException("Invalid named pipe name");
        }

        private TokenImpersonationLevel TokenImpersonationLevel { get; set; } = TokenImpersonationLevel.Impersonation;

        public override DPIPE_TYPE Type => DPIPE_TYPE.NAMED_PIPE;

        public int MaxInstances { get; set; } = 1;

        public int DefaultTimeout { get; set; }

        public bool UseRemote{  get; set; } = true;

        private void ReadLoop()
        {
            if (_readPipeStream == null || _writePipeStream == null)
                return;

            bool disconnection = false;

            if (_mode == DPIPE_MODE.INNITIATOR)
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
                _bytesToRead = header.DataSize;
                var command = PacketBuilder.GetCommand(header.ServiceCode);

                if (header.IsService())
                {
                    ServicePacketReceived(header);

                    if (command == Constants.SERVICE_CODE_DISCONNECTED)
                    {
                        disconnection = true;
                        break;
                    }

                    else if (command == Constants.SERVICE_CODE_DISCONNECT)
                    {
                        _packetPuilder.PrepareServiceHeader(Constants.SERVICE_CODE_DISCONNECTED);
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
            if (_mode == DPIPE_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new NamedPipeServerStream(PipeDirection.In, false, false, readPipeSafePipeHandle);
            _writePipeStream = new NamedPipeServerStream(PipeDirection.Out, false, false, writePipeSafePipeHandle);

            _listening = true;
            _mode = DPIPE_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public void Start(int maxNumberOfServerInstance, PipeOptions pipeOptions = PipeOptions.None)
        {
            if (_mode == DPIPE_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DPIPE_MODE.CLIENT)
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
            _mode = DPIPE_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override void Start()
        {
            Start(1, PipeOptions.None);
        }

        public void SetAccessControl(PipeSecurity pipeSecurity)
        {
            if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Cannot set Pipe Security properties in client mode");

            if (_mode == DPIPE_MODE.UNSTARTED)
                throw new Exception("Cannot set Pipe Security properties in unstarted mode");

            if (_readPipeStream == null)
                return;

            NamedPipeServerStream _readStreamPipeServer = (NamedPipeServerStream)_readPipeStream;
            _readStreamPipeServer.SetAccessControl(pipeSecurity);
        }
        public override IDPipeHandle GetHandle()
        {
            if (_mode == DPIPE_MODE.UNSTARTED)
                throw new Exception("Unable to get handle of unstarded dpipe");

            if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Unable to get handle in client mode");

            if (_mode == DPIPE_MODE.INNICIATOR_CLIENT ||  _mode == DPIPE_MODE.INNITIATOR)
            {
                if (UseRemote)
                    return new DPipeNamedHandle(Environment.MachineName, _name);
                else
                    return new DPipeNamedHandle(".", _name);
            }
               
            else
                throw new Exception("Undefined DPipe mode");
        }
        public override string GetHandleString()
        {
            return GetHandle().AsString();
        }
        public void Connect(DPipeNamedHandle pipeHandle, byte[] connectData = null)
        {
            if (_mode == DPIPE_MODE.CLIENT || _mode == DPIPE_MODE.INNITIATOR)
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

            //readNamedPipeClientStream.Connect();
            writeNamedPipeClientStream.Connect();
            readNamedPipeClientStream.Connect();

            _mode = DPIPE_MODE.CLIENT;
            _listening = true;
            _isAlive = true;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();

            int connectDataSize = connectData == null ? 0 : connectData.Length;

            _packetPuilder.PrepareServiceHeader(Constants.SERVICE_CODE_CONNECT, connectDataSize);
            _packetPuilder.WriteHeader(_writePipeStream);

            if (connectData != null && connectDataSize > 0)
                WriteRaw(connectData, 0, connectDataSize);
        }
        public override void Connect(IDPipeHandle pipeHandle, byte[] connectData)
        {
            if (_mode == DPIPE_MODE.CLIENT || _mode == DPIPE_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is null");

            DPipeNamedHandle namedHandle = (DPipeNamedHandle)pipeHandle;
            Connect(namedHandle, connectData);
        }
        public override void Connect(IDPipeHandle pipeHandle)
        {
            Connect(pipeHandle, null);
        }
        public override void Connect(string handleString, byte[] data)
        {
            var namedHandle = DPipeNamedHandle.Create(handleString);
            Connect(namedHandle, data);
        }
        public override void Connect(string handleString)
        {
            var namedHandle = DPipeNamedHandle.Create(handleString);
            Connect(namedHandle, null);
        }
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (_readPipeStream == null)
                return 0;

            int cycles = count / 65536;
            int restBytes = count % 65536;

            _bytesRead = 0;

            for (int i = 0; i < cycles; i++)
            {
                _bytesRead += _readPipeStream.Read(buffer, 65536 * i + offset, 65536);
            }

            if (restBytes > 0)
            {
                _bytesRead += _readPipeStream.Read(buffer, cycles * 65536 + offset, restBytes);
            }

            _bytesToRead -= _bytesRead;

            return _bytesRead;
        }
        public override void WriteRaw(byte[] buffer, int offset, int count)
        {
            if (_writePipeStream == null)
                return;

            int cycles = count / 65536;
            int restBytes = count % 65536;

            for (int i = 0; i < cycles; i++)
            {
                _writePipeStream.Write(buffer, 65536 * i + offset, 65536);
            }

            if (restBytes > 0)
            {
                _writePipeStream.Write(buffer, cycles * 65536 + offset, restBytes);
            }
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
            Write(Constants.SERVICE_CODE_RAW_CLIENT, buffer, offset, count);
        }
        public override void WritePacketHeader(PacketHeader header)
        {
            if (_writePipeStream == null)
                return;

            _packetPuilder.PrepareClientHeader(header.ServiceCode, header.DataSize);
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
    }
}
