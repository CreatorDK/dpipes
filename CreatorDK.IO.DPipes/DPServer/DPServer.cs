using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public interface IDPServer
    {
        DPipe Pipe { get; }

        void SendResponse(DPReceivedRequest request, DPClientResponse response);
    }

    public class DPServer : DPMessangerBase, IDPServer
    {
        public DPServer(DPipe dpipe, bool handleAsync, Encoding encoding = null, uint stringBufferSize = 4096) : 
            base(dpipe, encoding, stringBufferSize)
        {
            _handleAsync = handleAsync;

            _dpipe.OnOtherSideConnectCallback = OnClientConnectInner;
            _dpipe.OnOtherSideDisconnectCallback = OnClientDisonnectInner;
            _dpipe.OnPacketHeaderReceivedCallback = OnDataReceived;
        }

        ~DPServer()
        {
            _dpipe.OnOtherSideConnectCallback = null;
            _dpipe.OnOtherSideDisconnectCallback = null;
            _dpipe.OnPacketHeaderReceivedCallback = null;
        }

        private bool _handleAsync;
        private byte[] _pBufferRequest = new byte[Constants.DP_REQUEST_SIZE];
        private byte[] _pBufferResponse = new byte[Constants.DP_RESPONSE_SIZE];

        private Dictionary<int, DPHandler> _handlers = new Dictionary<int, DPHandler>();
        private Mutex _handlerMutex = new Mutex(false);

        public Action<PacketHeader> OnClientConnect;
        public Action<PacketHeader> OnClientDisconnect;

        private int _maxDescriptorMemoryAllocation = 33554432;  //32 MB
        private int _maxRequestMemoryAllocation = 33554432;     //32 MB

        public int MaxDescriptorMemoryAllocation
        {
            get => _maxDescriptorMemoryAllocation;
            set => _maxDescriptorMemoryAllocation = value;
        }
        public int MaxDescriptorMemoryAllocationMB
        {
            get => _maxDescriptorMemoryAllocation / 1048576;
            set => _maxDescriptorMemoryAllocation = value * 1048576;
        }

        public int MaxRequestMemoryAllocation
        {
            get => _maxRequestMemoryAllocation;
            set => _maxRequestMemoryAllocation = value;
        }
        public int MaxRequestMemoryAllocationMB
        {
            get => _maxRequestMemoryAllocation / 1048576;
            set => _maxRequestMemoryAllocation = value * 1048576;
        }

        public DPipe Pipe => _dpipe;

        private void OnClientConnectInner(PacketHeader header)
        {
            OnClientConnect?.Invoke(header);
        }
        private void OnClientDisonnectInner(PacketHeader header)
        {
            OnClientDisconnect?.Invoke(header);
        }
        private void OnDataReceived(PacketHeader header)
        {
            var command = header.DataCode;

            //ReadData from Data Request
            if (command == Constants.DP_REQUEST)
            {
                if (header.DataSize < Constants.DP_REQUEST_SIZE)
                    throw new Exception("Received broken request");

                _dpipe.Read(_pBufferRequest, 0, (int)Constants.DP_REQUEST_SIZE);
                var requestHeader = GetRequestHeader();

                bool descriptorAllocated;
                int descriptorSize = requestHeader.DescriptorSize;
                byte[] descriptor = null;

                if (descriptorSize <= _maxDescriptorMemoryAllocation)
                {
                    descriptor = new byte[descriptorSize];
                    _dpipe.Read(descriptor, 0, descriptorSize);
                    descriptorAllocated = true;
                }
                else
                    descriptorAllocated = false;

                bool dataAllocated;
                int dataSize = header.DataSize - (int)Constants.DP_RESPONSE_SIZE - descriptorSize;
                byte[] data = null;

                if (descriptorAllocated && dataSize <= _maxRequestMemoryAllocation)
                {
                    data = new byte[dataSize];
                    _dpipe.Read(data, 0, dataSize);
                    dataAllocated = true;
                }
                else
                    dataAllocated = false;


                //DPClientRequest request = new DPClientRequest(requestHeader.Guid, requestHeader.Code, requestHeader.DataType, data);
                DPReceivedRequest request = new DPReceivedRequest(
                    requestHeader.Guid,
                    requestHeader.Code,
                    requestHeader.DescriptorType,
                    requestHeader.DescriptorSize,
                    descriptorAllocated,
                    descriptor,
                    requestHeader.DataType,
                    dataSize,
                    dataAllocated,
                    data,
                    this
                    );


                if (_handleAsync)
                    Task.Run(() => OnRequestReceived(requestHeader, request));
                else
                    OnRequestReceived(requestHeader, request);

                return;
            }

            bool isStringData = (command & 0x01) > 0;

            //If data is string
            if (isStringData)
            {
                string message = GetString(header);

                if (_handleAsync)
                {
                    Task.Run(() => OnMessageStringReceivedInner(header, message));
                }
                else
                {
                    OnMessageStringReceivedInner(header, message);
                }
            }
            //If data is binary
            else
            {
                byte[] messageData = _dpipe.Read(header);

                if (_handleAsync)
                {
                    Task.Run(() => OnMessageDataReceivedInner(header, messageData));
                }
                else
                {
                    OnMessageDataReceivedInner(header, messageData);
                }
            }
        }

        private void OnRequestReceived(DPRequestHeader requestHeader, DPReceivedRequest request)
        {
            bool handlerExists = false;

            DPHandler functionDelegate = null;

            _handlerMutex.WaitOne();
            if (_handlers.ContainsKey(requestHeader.Code))
            {
                handlerExists = true;
                functionDelegate = _handlers[requestHeader.Code];
            }
            _handlerMutex.ReleaseMutex();

            if (functionDelegate != null)
                functionDelegate.Invoke(request);

            if (!handlerExists)
            {
                var respopnse = request.CreateResponse();
                respopnse.Code = (int)Constants.DP_HANDLER_NOT_FOUND;
                SendResponse(request, respopnse);
            }
        }
        private void PrepareResponce(DPClientResponse response, Guid guid)
        {
            byte[] guidBytes = guid.ToByteArray();
            byte[] codeBytes = BitConverter.GetBytes(response.Code);
            byte[] dataTypeBytes = BitConverter.GetBytes(response.DataType);
            byte[] descriptinTypeBytes = BitConverter.GetBytes(response.DescriptionType);
            byte[] descriptinSizeBytes = BitConverter.GetBytes(response.DescriptionSize);

            Buffer.BlockCopy(guidBytes, 0, _pBufferResponse, 0, guidBytes.Length);
            Buffer.BlockCopy(codeBytes, 0, _pBufferResponse, 16, codeBytes.Length);
            Buffer.BlockCopy(dataTypeBytes, 0, _pBufferResponse, 20, dataTypeBytes.Length);
            Buffer.BlockCopy(descriptinTypeBytes, 0, _pBufferResponse, 24, descriptinTypeBytes.Length);
            Buffer.BlockCopy(descriptinSizeBytes, 0, _pBufferResponse, 28, descriptinSizeBytes.Length);
        }
        private DPRequestHeader GetRequestHeader()
        {
            byte[] guidBytes = new byte[16];
            Buffer.BlockCopy(_pBufferRequest, 0, guidBytes, 0, 16);

            return new DPRequestHeader()
            {
                //Guid = new Guid(_pBufferRequest),
                Guid = new Guid(guidBytes),
                Code = BitConverter.ToInt32(_pBufferRequest, 16),
                DataType = BitConverter.ToInt32(_pBufferRequest, 20),
                DescriptorType = BitConverter.ToInt32(_pBufferRequest, 24),
                DescriptorSize = BitConverter.ToInt32(_pBufferRequest, 28),
            };
        }
        public void SendResponse(DPReceivedRequest request, DPClientResponse response)
        {
            _mutexWritePipe.WaitOne();

            PrepareResponce(response, request.Guid);
            int dataLen = _pBufferResponse.Length + (response.Data == null ? 0 : response.Data.Length);
            PacketHeader packetHeader = new PacketHeader(dataLen, Constants.DP_RESPONSE);
            _dpipe.WritePacketHeader(packetHeader);
            _dpipe.WriteRaw(_pBufferResponse, 0, (int)Constants.DP_RESPONSE_SIZE);

            if (response.Data != null && response.Data.Length > 0) 
                _dpipe.WriteRaw(response.Data, 0, response.Data.Length);

            _mutexWritePipe.ReleaseMutex();
        }
        public void SetHandler(int code, DPHandler handler)
        {
            _handlerMutex.WaitOne();
                _handlers.Add(code, handler);
            _handlerMutex.ReleaseMutex();
        }
        public void RemoveHandler(int code)
        {
            _handlerMutex.WaitOne();
            _handlers.Remove(code);
            _handlerMutex.ReleaseMutex();
        }
        public void Start()
        {
            _dpipe.Start();
        }
        public void Disconnect(byte[] data = null)
        {
            _dpipe.Disconnect(data);
        }
    }
}
