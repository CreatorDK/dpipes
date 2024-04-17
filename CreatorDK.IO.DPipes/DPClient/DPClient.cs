using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public interface IDPClient
    {

    }

    public class DPClient : DPMessangerBase, IDPClient
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

        public Action<PacketHeader> OnServerDisconnect;

        private int _maxDescriptorMemoryAllocation = 33554432; //32 MB
        private int _maxResponseMemoryAllocation = 33554432;   //32 MB

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

        public int MaResponseMemoryAllocation
        {
            get => _maxResponseMemoryAllocation;
            set => _maxResponseMemoryAllocation = value;
        }
        public int MaxResponseMemoryAllocationMB
        {
            get => _maxResponseMemoryAllocation / 1048576;
            set => _maxResponseMemoryAllocation = value * 1048576;
        }

        private void OnServerDisconnectInner(PacketHeader header)
        {
            OnServerDisconnect?.Invoke(header);
        }

        private void OnDataReceived(PacketHeader header)
        {
            var command = header.DataCode;

            //ReadData from Data Response
            if (command == Constants.DP_RESPONSE)
            {
                OnResponseReceived(header);
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

        private void OnResponseReceived(PacketHeader header)
        {
            if (header.DataSize < Constants.DP_RESPONSE_SIZE)
                throw new Exception("Received broken response");

            _dpipe.Read(_pBufferResponse, 0, (int)Constants.DP_RESPONSE_SIZE);
            var responseHeader = GetResponseHeader();

            bool descriptorAllocated;
            int descriptorSize = responseHeader.DescriptorSize;
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

            if (descriptorAllocated && dataSize <= _maxResponseMemoryAllocation)
            {
                data = new byte[dataSize];
                _dpipe.Read(data, 0, dataSize);
                dataAllocated = true;
            }
            else
                dataAllocated = false;

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
                //requestRecord.Response = new DPResponse(responseHeader.Code, responseHeader.DataType, data); ;
                requestRecord.Response = new DPResponse(responseHeader.Code,
                    responseHeader.DescriptorType,
                    responseHeader.DescriptorSize,
                    descriptorAllocated,
                    descriptor,
                    responseHeader.DataType,
                    dataSize,
                    dataAllocated,
                    data,
                    _dpipe);
                requestRecord.IsSucess = true;
            }

            _requestRecordListMutex.ReleaseMutex();
        }

        private DPResponseHeader GetResponseHeader()
        {
            byte[] guidBytes = new byte[16];
            Buffer.BlockCopy(_pBufferResponse, 0, guidBytes, 0, 16);

            return new DPResponseHeader()
            {
                //Guid = new Guid(_pBufferResponse),
                Guid = new Guid(guidBytes),
                Code = BitConverter.ToInt32(_pBufferResponse, 16),
                DataType = BitConverter.ToInt32(_pBufferResponse, 20),
                DescriptorType = BitConverter.ToInt32(_pBufferResponse, 24),
                DescriptorSize = BitConverter.ToInt32(_pBufferResponse, 28),
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
            byte[] descriptorTypeBytes = BitConverter.GetBytes(request.DescriptorType);
            byte[] descriptorSizeBytes = BitConverter.GetBytes(request.DescriptorSize);

            Buffer.BlockCopy(guidBytes, 0, _pBufferRequest, 0, guidBytes.Length);
            Buffer.BlockCopy(codeBytes, 0, _pBufferRequest, 16, codeBytes.Length);
            Buffer.BlockCopy(dataTypeBytes, 0, _pBufferRequest, 20, dataTypeBytes.Length);
            Buffer.BlockCopy(descriptorTypeBytes, 0, _pBufferRequest, 24, descriptorTypeBytes.Length);
            Buffer.BlockCopy(descriptorSizeBytes, 0, _pBufferRequest, 28, descriptorSizeBytes.Length);
        }

        public DPResponse SendRequest(DPRequest request, int millisecondsTimeout = 0)
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
                response = new DPResponse((int)Constants.DP_REQUEST_TIMEOUT, 0, 0, false, null, 0, 0, false, null, _dpipe );

            if (response is null)
                throw new ArgumentNullException("DPResponse", "Send method return null response");
            else
                return response;
        }

        public DPResponse SendRequest(int code, int dataType, byte[] data, int millisecondsTimeout = 0)
        {
            return SendRequest(new DPRequest() { Code = code, DataType = dataType, Data = data }, millisecondsTimeout);
        }
        public DPResponse SendRequest(int code, byte[] data, int millisecondsTimeout = 0)
        {
            return SendRequest(new DPRequest() { Code = code, DataType = 0, Data = data }, millisecondsTimeout);
        }

        public Task<DPResponse> SendAsync(DPRequest request, int millisecondsTimeout = 0)
        {
            return Task.Run(() => SendRequest(request, millisecondsTimeout));
        }

        public Task<DPResponse> SendAsync(int code, int dataType, byte[] data, int millisecondsTimeout = 0)
        {
            return Task.Run(() => SendRequest(code, dataType, data, millisecondsTimeout));
        }

        public Task<DPResponse> SendAsync(int code, byte[] data, int millisecondsTimeout = 0)
        {
            return Task.Run(() => SendRequest(code, data, millisecondsTimeout));
        }

        public void Connect(IDPipeHandle handle, byte[] connectData)
        {
            _dpipe.Connect(handle, connectData);
        }
        public void Connect(IDPipeHandle pipeHandle, string connectMessage = null, Encoding encoding = null)
        {
            if (string.IsNullOrEmpty(connectMessage))
                _dpipe.Connect(pipeHandle);
            else
            {
                var encodingCurrent = encoding == null ? Encoding : encoding;
                var data = encodingCurrent.GetBytes(connectMessage);
                var encodingCode = GetEncodingCode(encodingCurrent);
                _dpipe.Connect(pipeHandle, data, encodingCode);
            }
        }
        public void Connect(IDPipeHandle handle)
        {
            _dpipe.Connect(handle);
        }
        public void Connect(string handleString, byte[] connectData)
        {
            _dpipe.Connect(handleString, connectData);
        }
        public void Connect(string pipeHandleString, string connectMessage = null, Encoding encoding = null)
        {
            if (string.IsNullOrEmpty(connectMessage))
                _dpipe.Connect(pipeHandleString);
            else
            {
                var encodingCurrent = encoding == null ? Encoding : encoding;
                var data = encodingCurrent.GetBytes(connectMessage);
                var encodingCode = GetEncodingCode(encodingCurrent);
                _dpipe.Connect(pipeHandleString, data, encodingCode);
            }
        }
        public void Connect(string handleString)
        {
            _dpipe.Connect(handleString);
        }
    }
}
