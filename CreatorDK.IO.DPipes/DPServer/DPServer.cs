using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public delegate void DataReceivedHandler(byte[] data);

    public class DPServer : DPipeMessangerBase
    {
        public DPServer(DPipe dpipe, bool handleAsync, Encoding encoding = null, uint stringBufferSize = 4096) : 
            base(dpipe, encoding, stringBufferSize)
        {
            _handleAsync = handleAsync;

            _dpipe.OnClientConnectCallback = OnClientConnectInner;
            _dpipe.OnOtherSideDisconnectCallback = OnClientDisonnectInner;
            _dpipe.OnPacketHeaderReceivedCallback = OnDataReceived;
        }

        ~DPServer()
        {
            _dpipe.OnClientConnectCallback = null;
            _dpipe.OnOtherSideDisconnectCallback = null;
            _dpipe.OnPacketHeaderReceivedCallback = null;
        }

        private bool _handleAsync;
        private byte[] _pBufferRequest = new byte[Constants.DP_REQUEST_SIZE];
        private byte[] _pBufferResponse = new byte[Constants.DP_RESPONSE_SIZE];

        private Dictionary<int, DPHandler> _handlers = new Dictionary<int, DPHandler>();
        private Mutex _handlerMutex = new Mutex(false);

        public DataReceivedHandler OnClientConnect;
        public DataReceivedHandler OnClientDisconnect;

        private void OnClientConnectInner(PacketHeader header)
        {
            byte[] data = _dpipe.Read(header);

            OnClientConnect?.Invoke(data);
        }
        private void OnClientDisonnectInner(PacketHeader header)
        {
            byte[] data = _dpipe.Read(header);

            OnClientDisconnect?.Invoke(data);
        }
        private void OnDataReceived(PacketHeader header)
        {
            int command = (int)header.Command;
            bool isStringData = (command & 0x01) > 0;

            //If data is string
            if (isStringData)
            {
                string message = GetStringFromPipe(header.DataSize);

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
                //ReadData from Data Request
                if (command == 0)
                {
                    if (header.DataSize < Constants.DP_REQUEST_SIZE)
                        throw new Exception("Received broken request");

                    _dpipe.Read(_pBufferRequest, 0, (int)Constants.DP_REQUEST_SIZE);
                    var requestHeader = GetRequestHeader();

                    int dataSize = header.DataSize - (int)Constants.DP_REQUEST_SIZE;

                    byte[] data = null;

                    if (dataSize > 0)
                    {
                        data = new byte[dataSize];
                        _dpipe.Read(data, 0, dataSize);
                    }

                    if (_handleAsync)
                    {
                        Task.Run(() => OnRequestReceived(requestHeader, data));
                    }
                    else
                    {
                        OnRequestReceived(requestHeader, data);
                    }

                }
                //ReadData from Data Message
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
        }
        private void OnRequestReceived(DPRequestHeader requestHeader, byte[] data)
        {
            DPClientRequest request = new DPClientRequest(requestHeader.Guid, requestHeader.Code, requestHeader.DataType, data);

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

            System.Buffer.BlockCopy(guidBytes, 0, _pBufferResponse, 0, guidBytes.Length);
            System.Buffer.BlockCopy(codeBytes, 0, _pBufferResponse, guidBytes.Length, codeBytes.Length);
            System.Buffer.BlockCopy(dataTypeBytes, 0, _pBufferResponse, guidBytes.Length + codeBytes.Length, dataTypeBytes.Length);
        }
        private DPRequestHeader GetRequestHeader()
        {
            //byte[] guidBytes = new byte[16];
            //System.Buffer.BlockCopy(_pBufferResponse, 0, guidBytes, 0, 16);

            return new DPRequestHeader()
            {
                Guid = new Guid(_pBufferRequest),
                //Guid = new Guid(guidBytes),
                Code = BitConverter.ToInt32(_pBufferRequest, 16),
                DataType = BitConverter.ToInt32(_pBufferRequest, 20),
            };
        }
        public void SendResponse(DPClientRequest request, DPClientResponse response)
        {
            _mutexWritePipe.WaitOne();

            PrepareResponce(response, request.Guid);
            int dataLen = _pBufferResponse.Length + (response.Data == null ? 0 : response.Data.Length);
            PacketHeader packetHeader = new PacketHeader(dataLen, Constants.DP_REQUEST);
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
