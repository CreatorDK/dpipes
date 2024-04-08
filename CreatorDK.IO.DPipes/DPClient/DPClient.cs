using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public class DPClient : DPipeMessangerBase
    {
        public DPClient(DPipe dpipe, bool handleAsync, Encoding encoding = null, uint stringBufferSize = 4096) : 
            base(dpipe, encoding, stringBufferSize)
        {
            _handleAsync = handleAsync;

            _dpipe.OnOtherSideDisconnectCallback = OnServerDisconnectInner;
            _dpipe.OnPacketHeaderReceivedCallback = OnDataReceived;
        }

        ~DPClient()
        {
            _dpipe.OnOtherSideDisconnectCallback = null;
            _dpipe.OnPacketHeaderReceivedCallback = null;
        }

        private bool _handleAsync;
        private byte[] _pBufferRequest = new byte[Constants.DP_REQUEST_SIZE];
        private byte[] _pBufferResponse = new byte[Constants.DP_RESPONSE_SIZE];

        private List<DPRequestRecord> _requestRecordList = new List<DPRequestRecord>();
        private Mutex _requestRecordListMutex = new Mutex(false);

        public DataReceivedHandler OnServerDisconnect;

        private void OnServerDisconnectInner(PacketHeader header)
        {
            byte[] data = _dpipe.Read(header);

            OnServerDisconnect?.Invoke(data);
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
                //ReadData from Data Response
                if (command == 0)
                {
                    OnResponseReceived(header);
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

        private void OnResponseReceived(PacketHeader header)
        {
            if (header.DataSize < Constants.DP_RESPONSE_SIZE)
                throw new Exception("Received broken response");

            _dpipe.Read(_pBufferResponse, 0, (int)Constants.DP_RESPONSE_SIZE);
            var responseHeader = GetResponseHeader();

            int dataSize = header.DataSize - (int)Constants.DP_RESPONSE_SIZE;

            byte[] data = null;

            if (dataSize > 0)
            {
                data = new byte[dataSize];
                _dpipe.Read(data, 0, dataSize);
            }

            DPRequestRecord requestRecord = null;

            _requestRecordListMutex.WaitOne();

            foreach (var requesRecordCurent in _requestRecordList)
            {
                if (requesRecordCurent.Guid.Equals(responseHeader.Guid))
                {
                    requestRecord = requesRecordCurent;
                    break;
                }
            }

            if (requestRecord != null)
            {
                DPResponse response = new DPResponse(responseHeader.Code, responseHeader.DataType, data);
                requestRecord.Response = response;
                requestRecord.IsSucess = true;
            }

            _requestRecordListMutex.ReleaseMutex();
        }

        private DPResponseHeader GetResponseHeader()
        {
            //byte[] guidBytes = new byte[16];
            //System.Buffer.BlockCopy(_pBufferResponse, 0, guidBytes, 0, 16);

            return new DPResponseHeader()
            {
                Guid = new Guid(_pBufferResponse),
                //Guid = new Guid(guidBytes),
                Code = BitConverter.ToInt32(_pBufferResponse, 16),
                DataType = BitConverter.ToInt32(_pBufferResponse, 20),
            };
        }

        private void WaitRequestLoop(DPRequestRecord requestRecord, int millisecondsTimeout = 0)
        {
            Stopwatch stopWatch = millisecondsTimeout > 0 ? new Stopwatch() : null;
            stopWatch?.Start();

            while (!requestRecord.IsSucess)
            {
                Thread.Sleep(1);

                if (stopWatch != null)
                {
                    if (stopWatch.Elapsed.TotalMilliseconds > millisecondsTimeout)
                        break;
                }
            }
        }

        private void PrepareRequestRecord(DPRequest request, DPRequestRecord requestRecord)
        {
            byte[] guidBytes = requestRecord.Guid.ToByteArray();
            byte[] codeBytes = BitConverter.GetBytes(request.Code);
            byte[] dataTypeBytes = BitConverter.GetBytes(request.DataType);

            System.Buffer.BlockCopy(guidBytes, 0, _pBufferRequest, 0, guidBytes.Length);
            System.Buffer.BlockCopy(codeBytes, 0, _pBufferRequest, guidBytes.Length, codeBytes.Length);
            System.Buffer.BlockCopy(dataTypeBytes, 0, _pBufferRequest, guidBytes.Length + codeBytes.Length, dataTypeBytes.Length);
        }

        public DPResponse Send(DPRequest request, int millisecondsTimeout = 0)
        {
            Stopwatch stopwatch = new Stopwatch();
            stopwatch.Start();

            DPRequestRecord requestRecord = new DPRequestRecord()
            {
                Guid = Guid.NewGuid()
            };

            _requestRecordListMutex.WaitOne();
                _requestRecordList.Add(requestRecord);
            _requestRecordListMutex.ReleaseMutex();

            _mutexWritePipe.WaitOne();
                PrepareRequestRecord(request, requestRecord);
                int dataSize = (int)Constants.DP_REQUEST_SIZE + (request.Data == null ? 0 : request.Data.Length);

                PacketHeader packetHeader = new PacketHeader(dataSize, Constants.DP_REQUEST);
                _dpipe.WritePacketHeader(packetHeader);

                _dpipe.WriteRaw(_pBufferRequest, 0, (int)Constants.DP_REQUEST_SIZE);

                if (request.Data != null && request.Data.Length > 0)
                    _dpipe.WriteRaw(request.Data, 0, request.Data.Length);
            _mutexWritePipe.ReleaseMutex();

            TimeSpan time1 = stopwatch.Elapsed;

            WaitRequestLoop(requestRecord, millisecondsTimeout);

            var time2 = stopwatch.Elapsed;
            stopwatch.Stop();

            _requestRecordListMutex.WaitOne();
                _requestRecordList.Remove(requestRecord);
            _requestRecordListMutex.ReleaseMutex();

            DPResponse response;

            if (requestRecord.IsSucess)
            {
                response = requestRecord.Response;
                response?.SetSendDuraction(time1);
                response?.SetRequestDuraction(time2 - time1);
            }
                
            else
                response = new DPResponse((int)Constants.DP_REQUEST_TIMEOUT, 0, null);

            if (response is null)
                throw new ArgumentNullException("DPResponse", "Send method return null response");
            else
                return response;
        }

        public DPResponse Send(int code, int dataType, byte[] data, int millisecondsTimeout = 0)
        {
            return Send(new DPRequest() { Code = code, DataType = dataType, Data = data }, millisecondsTimeout);
        }
        public DPResponse Send(int code, byte[] data, int millisecondsTimeout = 0)
        {
            return Send(new DPRequest() { Code = code, DataType = 0, Data = data }, millisecondsTimeout);
        }

        public Task<DPResponse> SendAsync(DPRequest request, int millisecondsTimeout = 0)
        {
            return Task.Run(() => Send(request, millisecondsTimeout));
        }

        public Task<DPResponse> SendAsync(int code, int dataType, byte[] data, int millisecondsTimeout = 0)
        {
            return Task.Run(() => Send(code, dataType, data, millisecondsTimeout));
        }

        public Task<DPResponse> SendAsync(int code, byte[] data, int millisecondsTimeout = 0)
        {
            return Task.Run(() => Send(code, data, millisecondsTimeout));
        }

        public void Connect(IDPipeHandle handle, byte[] connectData)
        {
            _dpipe.Connect(handle, connectData);
        }
        public void Connect(IDPipeHandle handle, string connectMessage)
        {
            _dpipe.Connect(handle, connectMessage, _encoding);
        }
        public void Connect(IDPipeHandle handle)
        {
            _dpipe.Connect(handle);
        }
        public void Connect(string handleString, byte[] connectData)
        {
            _dpipe.Connect(handleString, connectData);
        }
        public void Connect(string handleString, string connectMessage)
        {
            _dpipe.Connect(handleString, connectMessage, _encoding);
        }
        public void Connect(string handleString)
        {
            _dpipe.Connect(handleString);
        }
        public void Disconnect(byte[] disconnectData)
        {
            _dpipe?.Disconnect(disconnectData);
        }

        public void Disconnect(string disconnectMessage)
        {
            _dpipe?.Disconnect(disconnectMessage, _encoding);
        }

        public void Disconnect()
        {
            _dpipe?.Disconnect();
        }
    }
}
