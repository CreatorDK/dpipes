using Microsoft.Win32.SafeHandles;
using System;
using System.IO;
using System.IO.Pipes;
using System.Threading;

namespace CreatorDK.IO.DPipes
{
    public class DPAnonymous : DPipe
    {
        SafePipeHandle _readClientSafeHandle = null;
        SafePipeHandle _writeClientSafeHandle = null;

        public DPAnonymous(string name = DPAnonymousHandle.DefaultPipeName, 
            int inBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT, 
            int outBufferSize = (int)Constants.DP_BUFFER_SIZE_DEFAULT) : 
                base(name, UInt32.MaxValue, inBufferSize, outBufferSize)
        { }

        public DPAnonymous(
            int inBufferSize,
            int outBufferSize) :
        base(DPAnonymousHandle.DefaultPipeName, UInt32.MaxValue, inBufferSize, outBufferSize)
        { }

        public override DP_TYPE Type => DP_TYPE.ANONYMOUS_PIPE;

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
        public void Start(
            SafePipeHandle sererReadHandle,
            SafePipeHandle clientWriteHandle,
            SafePipeHandle sererWriteHandle,
            SafePipeHandle clientReadHandle)
        {
            if (_mode == DP_MODE.INNITIATOR)
                throw new Exception("Pipe already created");
            else if (_mode == DP_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new AnonymousPipeClientStream(PipeDirection.In, sererReadHandle);
            _writePipeStream = new AnonymousPipeClientStream(PipeDirection.Out, sererWriteHandle);

            _readClientSafeHandle = clientReadHandle;
            _writeClientSafeHandle = clientWriteHandle;

            _listening = true;
            _mode = DP_MODE.INNICIATOR_CLIENT;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override void Start()
        {
            if (_mode == DP_MODE.INNITIATOR)
                throw new Exception("Pipe already created");

            else if (_mode == DP_MODE.CLIENT)
                throw new Exception("Cannot create pipe in client mode");

            _readPipeStream = new AnonymousPipeServerStream(PipeDirection.In, HandleInheritability.Inheritable, _inBufferSize);
            _writePipeStream = new AnonymousPipeServerStream(PipeDirection.Out, HandleInheritability.Inheritable, _outBufferSize);

            _listening = true;
            _mode = DP_MODE.INNITIATOR;

            _readThread = new Thread(ReadLoop);
            _readThread.Start();
        }
        public override IDPipeHandle GetHandle()
        {
            if (_mode == DP_MODE.UNSTARTED)
                throw new Exception("Unable to get handle of unstarded dpipe");

            if (_mode == DP_MODE.CLIENT)
                throw new Exception("Unable to get handle in client mode");

            if (_readPipeStream == null)
                throw new Exception("Anonymus Pipe Read Stream is null");

            if (_writePipeStream == null)
                throw new Exception("Anonymus Pipe Read Stream is null");

            if (_mode == DP_MODE.INNITIATOR)
            {
                AnonymousPipeServerStream _readPipeServerStream = (AnonymousPipeServerStream)_readPipeStream;
                AnonymousPipeServerStream _writePipeServerStream = (AnonymousPipeServerStream)_writePipeStream;

                return DPAnonymousHandle.Create(_readPipeServerStream.GetClientHandleAsString(), _writePipeServerStream.GetClientHandleAsString());
            }

            else if (_mode == DP_MODE.INNICIATOR_CLIENT)
            {
                if (_readClientSafeHandle == null)
                    throw new Exception("Unable to get DPipe read client handle");

                if (_writeClientSafeHandle == null)
                    throw new Exception("Unable to get DPipe write client handle");

                return DPAnonymousHandle.Create(
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
        public void ConnectAnonymus(DPAnonymousHandle pipeHandle, byte[] connectData = null, uint prefix = 0)
        {
            if (_mode == DP_MODE.CLIENT || _mode == DP_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is null");

            _readPipeStream = new AnonymousPipeClientStream(PipeDirection.In, pipeHandle.ReadHandle);
            _writePipeStream = new AnonymousPipeClientStream(PipeDirection.Out, pipeHandle.WriteHandle);

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

            int connectDataSize = connectData == null? 0 : connectData.Length;

            PacketHeader header = new PacketHeader(true, connectDataSize);
            header.ServiceCode = Constants.DP_SERVICE_CODE_CONNECT;
            header.ServicePrefix = prefix;

            _packetPuilder.PrepareHeader(header);
            _packetPuilder.WriteHeader(_writePipeStream);

            if (connectData != null && connectDataSize > 0)
                WriteRaw(connectData, 0, connectDataSize);
        }
        public override void Connect(IDPipeHandle pipeHandle, byte[] connectData = null, uint prefix = 0)
        {
            if (_mode == DP_MODE.CLIENT || _mode == DP_MODE.INNITIATOR)
                return;

            if (pipeHandle == null)
                throw new ArgumentException("Handle is invalid");

            DPAnonymousHandle anonymousHandle = (DPAnonymousHandle)pipeHandle;
            ConnectAnonymus(anonymousHandle, connectData, prefix);
        }
        public override void Connect(IDPipeHandle dPipeHandle)
        {
            Connect(dPipeHandle, null);
        }
        public override void Connect(string handleString, byte[] connectData, uint prefix = 0)
        {
            var anonymousHandle = DPAnonymousHandle.Create(handleString);
            ConnectAnonymus(anonymousHandle, connectData, prefix);
        }
        public override void Connect(string handleString)
        {
            Connect(handleString, null);
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

            if (_mode == DP_MODE.INNITIATOR)
            {
                AnonymousPipeServerStream _readPipeServerStream = (AnonymousPipeServerStream)_readPipeStream;
                AnonymousPipeServerStream _writePipeServerStream = (AnonymousPipeServerStream)_writePipeStream;

                _readPipeServerStream.DisposeLocalCopyOfClientHandle();
                _writePipeServerStream.DisposeLocalCopyOfClientHandle();
            }
            else if (_mode == DP_MODE.INNICIATOR_CLIENT)
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

            if (!_clientEmulating)
            {
                _readClientSafeHandle?.Dispose();
                _writeClientSafeHandle?.Dispose();
            }
        }

        public override DPipe CreateNewInstance()
        {
            return new DPAnonymous(_inBufferSize, _outBufferSize); ;
        }
    }
}
