#include "dpclient.h"

using namespace std;
using namespace crdk::dpipes;

typedef std::chrono::high_resolution_clock Time;

//DPResponse implementation
#pragma region DPResponse
	DPResponse::DPResponse(int code, int dataType, std::shared_ptr<HeapAllocatedData> data) {
		_code = code;
		_dataType = dataType;
		_data = data;
	}

	DPResponse::~DPResponse() {
	
	}

	int DPResponse::code() const {
		return _code;
	}

	int DPResponse::dataType() const {
		return _dataType;
	}

	void* DPResponse::data() const {
		return _data->data();
	}

	shared_ptr<HeapAllocatedData> DPResponse::getData() {
		return _data;
	}

	DWORD DPResponse::dataSize() const {
		return _data->size();
	}

	bool DPResponse::keepAllocatedMemory() {
		return _data->keepAllocatedMemory;
	}

	void DPResponse::keepAllocatedMemory(bool keepAllocatedMemory) {
		_data->keepAllocatedMemory = keepAllocatedMemory;
	}

	chrono::duration<float> DPResponse::sendDuraction() const {
		return _send_duraction;
	}

	chrono::duration<float> DPResponse::requestDuraction() const {
		return _request_duraction;
	}

	chrono::duration<float> DPResponse::totalDuraction() const {
		return _request_duraction + _send_duraction;
	}
#pragma endregion DPResponse

//DPClient implementation
#pragma region DPClient
	DPClient::DPClient(IDPipe* dpipe, bool handleAsync, DWORD nBufferStringSize) :
		IDPipeMessanger(dpipe, nBufferStringSize)
	{
		if (_dpipe == nullptr)
			return;

		_handleAsync = handleAsync;

		_nBufferRequestSize = DP_REQUEST_SIZE;
		_pBufferRequest = new char [DP_REQUEST_SIZE];

		_nBufferResponseSize = DP_RESPONSE_SIZE;
		_pBufferResponse = new char[DP_RESPONSE_SIZE];

		_dpipe->SetOtherSideDisconnectCallback([=](PacketHeader header) { OnServerDisconnecting(header); });
		_dpipe->SetPacketHeaderRecevicedCallback([=](PacketHeader header) { OnDataReceived(header); });
	}

	DPClient::~DPClient()
	{ 
		_dpipe->SetOtherSideDisconnectCallback(nullptr);
		_dpipe->SetPacketHeaderRecevicedCallback(nullptr);

		delete[] _pBufferRequest;
		delete[] _pBufferResponse;
	}

	void DPClient::OnServerDisconnecting(PacketHeader header) {
		DWORD dataSize = header.DataSize();
		DWORD nBytesRead;

		//_dpipe->Read(_pBufferRequest, DP_REQUEST_SIZE, &nBytesRead, NULL);
		auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

		if (dataSize > 0) {
			void* buffer = heapData->data();
			_dpipe->Read(buffer, dataSize, &nBytesRead, NULL);
		}

		if (_onServerDisconnect)
			_onServerDisconnect(heapData);
	}

	void DPClient::OnDataReceived(PacketHeader header) {

		DWORD nBytesRead;
		DWORD serviceCode = header.GetServiceCode();
		bool isStringData = serviceCode & 0x01;

		//If data is string
		if (isStringData) {

			if (_handleAsync) {
				auto messangerPtr = dynamic_cast<IDPipeMessanger*>(this);
				DPMessageStringReceivedAsyncData* messageData = new DPMessageStringReceivedAsyncData(messangerPtr, header, GetStringFromPipe(header.DataSize()));

				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageStringReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
			}
			else
				OnMessageStringReceived(header, GetStringFromPipe(header.DataSize()));
		}

		//If data is binary
		else {

			//ReadData from Response
			if (serviceCode == 0) {
				OnResponseReceived(header);
			}

			//ReadData from Message
			else {
				DWORD dataSize = header.DataSize();
				auto heapData = make_shared<HeapAllocatedData>(dataSize, keepMessageAllocatedMemory);

				if (dataSize > 0) {
					//_mutexReadPipe.lock();
					void* buffer = heapData.get()->data();
					_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
					//_mutexReadPipe.unlock();
				}
				if (_handleAsync) {
					DPRequestReceivedAsyncData* reqData = new DPRequestReceivedAsyncData;
					auto messangerPtr = dynamic_cast<IDPipeMessanger*>(this);
					DPMessageReceivedAsyncData* messageData = new DPMessageReceivedAsyncData(messangerPtr, header, heapData);

					DWORD _nReadThreadId;
					auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
				}
				else
					OnMessageReceived(header, heapData);
			}
		}
	}

	void DPClient::OnResponseReceived(PacketHeader& header) {
		DWORD dataSize = header.DataSize();

		if (dataSize < DP_RESPONSE_SIZE)
			throw exception("Received broken response");

		DWORD nBytesRead;
		DPResponseHeader respRecord;
		_dpipe->Read(_pBufferResponse, DP_RESPONSE_SIZE, &nBytesRead, NULL);
		FillResponseHeader(respRecord);

		dataSize = dataSize - DP_RESPONSE_SIZE;
		auto heapData = make_shared<HeapAllocatedData>(dataSize, keepResponseAllocatedMemory);

		if (dataSize > 0) {
			void* buffer = new char[dataSize];
			_dpipe->Read(buffer, dataSize, &nBytesRead, NULL);
		}

		bool requestFound = false;

		_requestListMutex.lock();

			for (DPRequestRecord* req : _requestList) {
				if (IsEqualGUID(req->guid, respRecord.guid)) {
					requestFound = true;
					shared_ptr<DPResponse> resp = shared_ptr<DPResponse>(new DPResponse(respRecord.code, respRecord.dataType, heapData));
					req->resp = resp;
					req->succes = true;
				}
			}

		_requestListMutex.unlock();

		if (!requestFound) {
			heapData->keepAllocatedMemory = false;
		}
	}

	void DPClient::WaitRequestLoop(DPRequestRecord* requsertRecord, unsigned int timeout) {

		auto timeBegin = Time::now();

		while (!requsertRecord->succes) {
			Sleep(1);

			if (timeout > 0) {
				chrono::duration<float> duraction = Time::now() - timeBegin;
				auto waitingTime = chrono::duration_cast<chrono::milliseconds>(duraction).count();

				if (waitingTime > timeout)
					break;
			}
		}
	}

	void DPClient::FillResponseHeader(const DPResponseHeader& responseRecord) const {
		memcpy_s((char*)&responseRecord.guid, sizeof(GUID), _pBufferResponse, sizeof(GUID));
		memcpy_s((char*)&responseRecord.code, sizeof(int), _pBufferResponse + sizeof(GUID), sizeof(int));
		memcpy_s((char*)&responseRecord.dataType, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int), sizeof(int));
	}

	void DPClient::PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord) {
		memcpy_s(_pBufferRequest, sizeof(GUID), (char*)&requestRecord->guid, sizeof(GUID));
		memcpy_s(_pBufferRequest + sizeof(GUID), sizeof(int), (char*)&request.code, sizeof(int));
		memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&request.dataType, sizeof(int));
	}

	void DPClient::ReceiveResponseAsync(LPVOID dataPtr) {

		DPRequestAsyncData* reqData = (DPRequestAsyncData*)dataPtr;
		DPClient* client = reqData->client;

		auto resp = client->Send(reqData->req, reqData->timeout);

		if (reqData->callback)
			reqData->callback(resp);

		delete reqData;
	}

	shared_ptr<DPResponse> DPClient::Send(const DPRequest& req, unsigned int timeout)
	{
		typedef std::chrono::high_resolution_clock Time;
		auto t0 = Time::now();

		DPRequestRecord* requestRecord = new DPRequestRecord();
		auto res = CoCreateGuid(&requestRecord->guid);

		_requestListMutex.lock();
			_requestList.push_front(requestRecord);
		_requestListMutex.unlock();

		_mutexWritePipe.lock();
			PrepareRequestRecord(req, requestRecord);
			DWORD nBytesWritten;
			DWORD packetSize = _nBufferRequestSize + req.dataSize;
			PacketHeader header(packetSize, DP_REQUEST);
			_dpipe->WritePacketHeader(header);
			_dpipe->WriteRaw(_pBufferRequest, _nBufferRequestSize, &nBytesWritten);

			if (req.dataSize > 0) {
				if (req.data != nullptr)
					_dpipe->WriteRaw(req.data, req.dataSize, &nBytesWritten);
				else
					throw exception("Unable to send request data: buffer is null");
			}
		_mutexWritePipe.unlock();
		auto t1 = Time::now();

		WaitRequestLoop(requestRecord, timeout);

		auto t2 = Time::now();

		_requestListMutex.lock();
			_requestList.remove(requestRecord);
		_requestListMutex.unlock();

		shared_ptr<DPResponse> response = nullptr;

		if (requestRecord->succes) {
			shared_ptr<DPResponse> response = shared_ptr<DPResponse>(requestRecord->resp);
			response->_send_duraction = t1 - t0;
			response->_request_duraction = t2 - t1;

			delete requestRecord;
			return response;
		}
		else {

			delete requestRecord;
			return make_shared<DPResponse>(DP_REQUEST_TIMEOUT, 0, nullptr);
		}
	}

	shared_ptr<DPResponse> DPClient::Send(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout)
	{
		DPRequest req;
		req.code = code;
		req.dataType = dataType;
		req.data = data;
		req.dataSize = dataSize;
		return Send(req, timeout);
	}

	shared_ptr<DPResponse> DPClient::Send(int code, void* data, DWORD dataSize, unsigned int timeout)
	{
		DPRequest req;
		req.code = code;
		req.dataType = DP_DATA_BINARY;
		req.data = data;
		req.dataSize = dataSize;
		return Send(req, timeout);
	}

	void DPClient::SendAsync(const DPRequest& req, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {
		
		DPRequestAsyncData* reqData = new DPRequestAsyncData();
		reqData->req = req;
		reqData->client = this;
		reqData->callback = receiveResponseCallback;
		reqData->timeout = timeout;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveResponseAsync, (LPVOID)reqData, 0, &_nReadThreadId);
	}

	void DPClient::SendAsync(int code, int dataType, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

		DPRequest req;
		req.code = code;
		req.dataType = dataType;
		req.data = data;
		req.dataSize = dataSize;

		DPRequestAsyncData* reqData = new DPRequestAsyncData();
		reqData->req = req;
		reqData->client = this;
		reqData->callback = receiveResponseCallback;
		reqData->timeout = timeout;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveResponseAsync, (LPVOID)reqData, 0, &_nReadThreadId);
	}

	void DPClient::SendAsync(int code, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

		DPRequest req;
		req.code = code;
		req.dataType = DP_DATA_BINARY;
		req.data = data;
		req.dataSize = dataSize;

		DPRequestAsyncData* reqData = new DPRequestAsyncData();
		reqData->req = req;
		reqData->client = this;
		reqData->callback = receiveResponseCallback;
		reqData->timeout = timeout;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveResponseAsync, (LPVOID)reqData, 0, &_nReadThreadId);
	}

	void DPClient::SetOnServerDisconnectCallback(function<void(std::shared_ptr<HeapAllocatedData> data)> function) {
		_onServerDisconnect = function;
	}

	void DPClient::FreeAllocatedMemory(void* data) {
		delete[] data;
	}

	bool DPClient::Connect(IDPipeHandle* pHandle) {
		return _dpipe->Connect(pHandle);
	}

	bool DPClient::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) {
		return _dpipe->Connect(pHandle, pConnectData, nConnectDataSize);
	}

	bool DPClient::Connect(const std::wstring handleString) {
		return _dpipe->Connect(handleString);
	}

	bool DPClient::Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) {
		return _dpipe->Connect(handleString, pConnectData, nConnectDataSize);
	}

	void DPClient::Disconnect() {
		_dpipe->Disconnect();
	}

	void DPClient::Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize) {
		_dpipe->Disconnect(pConnectData, nConnectDataSize);
	}

#pragma endregion DPClient