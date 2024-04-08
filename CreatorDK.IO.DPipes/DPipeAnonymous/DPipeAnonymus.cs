using Microsoft.Win32.SafeHandles;
using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;

namespace CreatorDK.IO.DPipes
{
    public class DPipeAnonymus : DPipe
    {
        SafePipeHandle _readClientSafeHandle = null;
        SafePipeHandle _writeClientSafeHandle = null;

        public DPipeAnonymus(string name = DPipeAnonymousHandle.DefaultPipeName, 
            int inBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT, 
            int outBufferSize = (int)Constants.PIPE_BUFFERLENGTH_DEFAULT) : 
                base(name, inBufferSize, outBufferSize)
        { }

        public DPipeAnonymus(
            int inBufferSize,
            int outBufferSize) :
        base(DPipeAnonymousHandle.DefaultPipeName, inBufferSize, outBufferSize)
        { }

        public override DPIPE_TYPE Type => DPIPE_TYPE.ANONYMUS_PIPE;

        private void ReadLoop()
        {
            if (_readPipeStream == null || _writePipeStream == null)
                return;

            bool disconnection = false;

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
        public void Start(
            SafePipeHandle sererReadHandle,
            SafePipeHandle clientWriteHandle,
            SafePipeHandle sererWriteHandle,
            SafePipeHandle clientReadHandle)
        {
            if (_mode == DPIPE_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new AnonymousPipeClientStream(PipeDirection.In, sererReadHandle);
            _writePipeStream = new AnonymousPipeClientStream(PipeDirection.Out, sererWriteHandle);

            _readClientSafeHandle = clientReadHandle;
            _writeClientSafeHandle = clientWriteHandle;

            _listening = true;
            _mode = DPIPE_MODE.INNICIATOR_CLIENT;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override void Start()
        {
            if (_mode == DPIPE_MODE.INNITIATOR)
                throw new Exception("Pipe already created");

            else if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new AnonymousPipeServerStream(PipeDirection.In, HandleInheritability.Inheritable, _inBufferSize);
            _writePipeStream = new AnonymousPipeServerStream(PipeDirection.Out, HandleInheritability.Inheritable, _outBufferSize);

            _listening = true;
            _mode = DPIPE_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override IDPipeHandle GetHandle()
        {
            if (_mode == DPIPE_MODE.UNSTARTED)
                throw new Exception("Unable to get handle of unstarded dpipe");

            if (_mode == DPIPE_MODE.CLIENT)
                throw new Exception("Unable to get handle in client mode");

            if (_readPipeStream == null)
                throw new Exception("Anonymus Pipe Read Stream is null");

            if (_writePipeStream == null)
                throw new Exception("Anonymus Pipe Read Stream is null");

            if (_mode == DPIPE_MODE.INNITIATOR)
            {
                AnonymousPipeServerStream _readPipeServerStream = (AnonymousPipeServerStream)_readPipeStream;
                AnonymousPipeServerStream _writePipeServerStream = (AnonymousPipeServerStream)_writePipeStream;

                return DPipeAnonymousHandle.Create(_readPipeServerStream.GetClientHandleAsString(), _writePipeServerStream.GetClientHandleAsString());
            }

            else if (_mode == DPIPE_MODE.INNICIATOR_CLIENT)
            {
                if (_readClientSafeHandle == null)
                    throw new Exception("Unable to get DPipe read client handle");

                if (_writeClientSafeHandle == null)
                    throw new Exception("Unable to get DPipe write client handle");

                return DPipeAnonymousHandle.Create(
                    _readClientSafeHandle.DangerousGetHandle().ToString(),
                    _writeClientSafeHandle.DangerousGetHandle().ToString()
                    );
            }

            else
                throw new Exception("Unable to get DPipe client handle. Undefined Server mode");
        }
        public override string GetHandleString()
        {
            return GetHandle().AsString();
        }
        public void Connect(DPipeAnonymousHandle pipeHandle, byte[] connectData = null)
        {
            if (_mode == DPIPE_MODE.CLIENT || _mode == DPIPE_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is null");

            _readPipeStream = new AnonymousPipeClientStream(PipeDirection.In, pipeHandle.ReadHandle);
            _writePipeStream = new AnonymousPipeClientStream(PipeDirection.Out, pipeHandle.WriteHandle);

            _mode = DPIPE_MODE.CLIENT;
            _listening = true;
            _isAlive = true;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();

            int connectDataSize = connectData == null? 0 : connectData.Length;

            _packetPuilder.PrepareServiceHeader(Constants.SERVICE_CODE_CONNECT, connectDataSize);
            _packetPuilder.WriteHeader(_writePipeStream);

            if (connectData != null && connectDataSize > 0)
                WriteRaw(connectData, 0, connectDataSize);
        }
        public override void Connect(IDPipeHandle pipeHandle, byte[] connectData = null)
        {
            if (_mode == DPIPE_MODE.CLIENT || _mode == DPIPE_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is invalid");

            DPipeAnonymousHandle anonymusHandle = (DPipeAnonymousHandle)pipeHandle;
            Connect(anonymusHandle, connectData);
        }
        public override void Connect(IDPipeHandle dPipeHandle)
        {
            Connect(dPipeHandle, null);
        }
        public override void Connect(string handleString, byte[] connectData)
        {
            var anonymusHandle = DPipeAnonymousHandle.Create(handleString);
            Connect(anonymusHandle, connectData);
        }
        public override void Connect(string handleString)
        {
            Connect(handleString, null);
        }
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (_readPipeStream == null)
                return 0;

            int bytesRead = _readPipeStream.Read(buffer, offset, count);

            _bytesRead += bytesRead;
            _bytesToRead -= bytesRead;

            return bytesRead;
        }
        public override void WriteRaw(byte[] buffer, int offset, int count)
        {
            if (_writePipeStream == null)
                return;

            _writePipeStream.Write(buffer, offset, count);
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

            if (_mode == DPIPE_MODE.INNITIATOR)
            {
                AnonymousPipeServerStream _readPipeServerStream = (AnonymousPipeServerStream)_readPipeStream;
                AnonymousPipeServerStream _writePipeServerStream = (AnonymousPipeServerStream)_writePipeStream;

                _readPipeServerStream.DisposeLocalCopyOfClientHandle();
                _writePipeServerStream.DisposeLocalCopyOfClientHandle();
            }
            else if (_mode == DPIPE_MODE.INNICIATOR_CLIENT)
            {
                _readClientSafeHandle?.Dispose();
                _writeClientSafeHandle?.Dispose();
            }
        }
        public override void DisconnectPipe()
        {
            //var readHandle= _readPipeStream?.SafePipeHandle;
            _readPipeStream?.Dispose();
            //readHandle?.Dispose();

            //var writeHandle = _writePipeStream?.SafePipeHandle;
            _writePipeStream?.Dispose();
            //writeHandle?.Dispose();

            _readClientSafeHandle?.Dispose();
            _writeClientSafeHandle?.Dispose();
        }
    }
}
