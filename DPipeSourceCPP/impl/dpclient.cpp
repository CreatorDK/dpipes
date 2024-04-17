#include "dpclient.h"

using namespace std;
using namespace crdk::dpipes;

typedef std::chrono::high_resolution_clock Time;

//DPResponse implementation
#pragma region DPResponse
	DPResponse::DPResponse(
		int code, 
		int descriptorType,
		DWORD descriptorSize,
		bool descriptorAllocated,
		std::shared_ptr<HeapAllocatedData> descriptor,
		int dataType, 
		DWORD dataSize,
		std::shared_ptr<HeapAllocatedData> data, 
		IDPipe* dpipe) {
		_code = code;
		_descriptorType = descriptorType;
		_descriptorSize = descriptorSize;
		_descriptorAllocated = descriptorAllocated;
		_descriptor = descriptor;
		_dataType = dataType;
		_dataSize = dataSize;
		_data = data;
		_dpipe = dpipe;
	}

	DPResponse::~DPResponse() {
	
	}

	int DPResponse::code() const {
		return _code;
	}

	bool DPResponse::descriptorAllocated() const {
		return _descriptorAllocated;
	}

	int crdk::dpipes::DPResponse::descriptorType() const {
		return _descriptorType;
	}

	DWORD DPResponse::descriptorSize() const {
		return _descriptorSize;
	}

	std::shared_ptr<HeapAllocatedData> crdk::dpipes::DPResponse::descriptor() {
		return _descriptor;
	}

	void* DPResponse::descriptorPtr() const {
		return _descriptor->data();
	}

	bool DPResponse::dataAllocated() const {
		return _dataAllocated;
	}

	int DPResponse::dataType() const {
		return _dataType;
	}

	DWORD DPResponse::dataSize() const {
		return _dataSize;
	}

	shared_ptr<HeapAllocatedData> DPResponse::data() {
		return _data;
	}

	void* DPResponse::dataPtr() const {
		return _data->data();
	}

	bool crdk::dpipes::DPResponse::keepDescriptor() {
		return _descriptor->keepAllocatedMemory;
	}

	void crdk::dpipes::DPResponse::keepDescriptor(bool keepAllocatedMemory) {
		_descriptor->keepAllocatedMemory = keepAllocatedMemory;
	}

	bool DPResponse::keepData() {
		return _data->keepAllocatedMemory;
	}

	void DPResponse::keepData(bool keepAllocatedMemory) {
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

	IDPipe* DPResponse::dpipe() const {
		return _dpipe;
	}
	void DPResponse::ReadDescriptor(bool keepAllocatedMemory) {

		if (!_descriptorAllocated && _descriptorSize > 0) {

			_descriptor.~shared_ptr();

			_descriptor = make_shared<HeapAllocatedData>(_descriptorSize, keepAllocatedMemory);
			_dpipe->Read(_descriptor->data(), _descriptorSize);
			_descriptorAllocated = true;
		}

	}
	void DPResponse::Read(bool keepAllocatedMemory) {

		ReadDescriptor(keepAllocatedMemory);

		if (!_dataAllocated && _dataSize > 0) {

			_data.~shared_ptr();

			_data = make_shared<HeapAllocatedData>(_dataSize, keepAllocatedMemory);
			_dpipe->Read(_data->data(), _dataSize);
			_dataAllocated = true;
		}
	}
#pragma endregion DPResponse

//DPClient implementation
#pragma region DPClient
	DPClient::DPClient(IDPipe* dpipe, bool handleAsync):
		IDPipeMessanger(dpipe)
	{
		_dpipe = dpipe;
		_handleResponseAsync = handleAsync;

		_nBufferRequestSize = DP_REQUEST_SIZE;
		_pBufferRequest = new char [DP_REQUEST_SIZE];

		_nBufferResponseSize = DP_RESPONSE_SIZE;
		_pBufferResponse = new char[DP_RESPONSE_SIZE];

		_dpipe->SetOtherSideDisconnectCallback([=](IDPipe* pipe, PacketHeader header) { OnServerDisconnecting(pipe, header); });
		_dpipe->SetPacketHeaderRecevicedCallback([=](IDPipe* pipe, PacketHeader header) { OnDataReceived(pipe, header); });
	}

	DPClient::~DPClient()
	{ 
		_dpipe->SetOtherSideDisconnectCallback({});
		_dpipe->SetPacketHeaderRecevicedCallback({});

		delete[] _pBufferRequest;
		delete[] _pBufferResponse;
	}

	void DPClient::OnServerDisconnecting(IDPipe* pipe, PacketHeader header) {
		if (_onServerDisconnect)
			_onServerDisconnect(pipe, header);
	}

	void DPClient::OnDataReceived(IDPipe* pipe, PacketHeader header) {

		DWORD nBytesRead;

		//Get data code ignoring first 8 bits (using for encoding)
		DWORD dataCode = header.GetDataCodeOnly();

		if (dataCode == DP_RESPONSE) {
			OnResponseReceived(header);
			return;
		}

		bool isStringData = dataCode & 0x01;

		//If data is string
		if (isStringData) {

			DWORD dataSize = header.DataSize();
			auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

			if (dataSize > 0) {
				void* buffer = heapData.get()->data();
				_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
			}

			if (_handleResponseAsync) {
				DPMessageReceivedAsyncData* messageData = new DPMessageReceivedAsyncData(this, header, heapData, pipe);

				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageStringReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
			}
			else
				OnMessageStringReceived(pipe, header, heapData);
		}

		//If data is binary
		else {
			DWORD dataSize = header.DataSize();
			auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

			if (dataSize > 0) {
				void* buffer = heapData.get()->data();
				_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
			}

			if (_handleResponseAsync) {
				DPMessageReceivedAsyncData* messageData = new DPMessageReceivedAsyncData(this, header, heapData, pipe);
				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
			}
			else
				OnMessageReceived(pipe, header, heapData);
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

		bool descriptorAllocated;
		DWORD descriptorSize = respRecord.descriptorSize;
		shared_ptr<HeapAllocatedData> descriptorData = nullptr;

		if (descriptorSize <= _maxDescriptorMemoryAllocation) {
			descriptorData = make_shared<HeapAllocatedData>(descriptorSize, _keepDescriptorMemoryAllocated);
			_dpipe->Read(descriptorData->data(), descriptorSize);
			descriptorAllocated = true;
		}
		else {
			descriptorData = make_shared<HeapAllocatedData>(0, _keepDescriptorMemoryAllocated);
			descriptorAllocated = false;
		}

		bool dataAllocated;
		dataSize = dataSize - DP_RESPONSE_SIZE - descriptorSize;
		shared_ptr<HeapAllocatedData> data = nullptr;

		if (descriptorAllocated && dataSize <= _maxResponseMemoryAllocation) {
			data = make_shared<HeapAllocatedData>(dataSize, _keepResponseMemoryAllocated);
			_dpipe->Read(data->data(), dataSize);
			dataAllocated = true;
		}
		else {
			data = make_shared<HeapAllocatedData>(0, false);
			dataAllocated = false;
		}

		bool requestFound = false;

		_requestListMutex.lock();

			for (DPRequestRecord* req : _requestList) {
				if (IsEqualGUID(req->guid, respRecord.guid)) {
					requestFound = true;
					shared_ptr<DPResponse> resp = make_shared<DPResponse>(
						respRecord.code,
						respRecord.descriptorType,
						descriptorSize,
						descriptorAllocated,
						descriptorData,
						respRecord.dataType,
						dataSize,
						data,
						_dpipe
					);
					req->resp = resp;
					req->succes = true;
				}
			}

		_requestListMutex.unlock();

		if (!requestFound) {
			descriptorData->keepAllocatedMemory = false;
			data->keepAllocatedMemory = false;
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
		memcpy_s((char*)&responseRecord.descriptorType, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int) * 2, sizeof(int));
		memcpy_s((char*)&responseRecord.descriptorSize, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int) * 3, sizeof(int));
	}

	void DPClient::PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord) {
		memcpy_s(_pBufferRequest, sizeof(GUID), (char*)&requestRecord->guid, sizeof(GUID));
		memcpy_s(_pBufferRequest + sizeof(GUID), sizeof(int), (char*)&request.code, sizeof(int));
		memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&request.dataType, sizeof(int));
		memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int) * 2, sizeof(int), (char*)&request.descriptorType, sizeof(int));
		memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int) * 3, sizeof(int), (char*)&request.descriptorSize, sizeof(int));
	}

	bool DPClient::keepDescriptorMemoryAllocated() {
		return _keepDescriptorMemoryAllocated;
	}

	void DPClient::keepDescriptorMemoryAllocated(bool value) {
		_keepDescriptorMemoryAllocated = value;
	}

	void DPClient::ReceiveResponseAsync(LPVOID dataPtr) {

		DPRequestAsyncData* reqData = (DPRequestAsyncData*)dataPtr;
		IDPClient* client = reqData->client;

		auto resp = client->SendRequest(reqData->req, reqData->timeout);

		if (reqData->callback)
			reqData->callback(resp);

		delete reqData;
	}

	bool DPClient::keepResponseMemoryAllocated() {
		return _keepResponseMemoryAllocated;
	}

	void DPClient::keepResponseMemoryAllocated(bool value) {
		_keepResponseMemoryAllocated = value;
	}

	DWORD DPClient::maxDescriptorMemoryAllocation() {
		return _maxDescriptorMemoryAllocation;
	}

	void DPClient::maxDescriptorMemoryAllocation(DWORD value) {
		_maxDescriptorMemoryAllocation = value;
	}

	DWORD DPClient::maxResponseMemoryAllocation() {
		return _maxResponseMemoryAllocation;
	}

	DWORD DPClient::maxResponseMemoryAllocationMB() {
		return _maxResponseMemoryAllocation / 1048576;
	}

	void DPClient::maxResponseMemoryAllocation(DWORD value) {
		_maxResponseMemoryAllocation = value;
	}

	void DPClient::maxResponseMemoryAllocationMB(DWORD value) {
		_maxResponseMemoryAllocation = value * 1048576;
	}

	shared_ptr<DPResponse> DPClient::SendRequest(const DPRequest& req, unsigned int timeout)
	{
		typedef std::chrono::high_resolution_clock Time;
		auto t0 = Time::now();

		DPRequestRecord* requestRecord = new DPRequestRecord();
		auto res = CoCreateGuid(&requestRecord->guid);

		_requestListMutex.lock();
			_requestList.push_front(requestRecord);
		_requestListMutex.unlock();

		_mutexWrite.lock();
			PrepareRequestRecord(req, requestRecord);
			DWORD nBytesWritten;
			DWORD packetSize = _nBufferRequestSize + req.descriptorSize + req.dataSize;

			PacketHeader header(false, packetSize);
			header.SetDataCode(DP_REQUEST);

			_dpipe->WritePacketHeader(header);
			_dpipe->WriteRaw(_pBufferRequest, _nBufferRequestSize, &nBytesWritten);

			if (req.descriptorSize > 0) {
				if (req.descriptor != nullptr)
					_dpipe->WriteRaw(req.descriptor, req.descriptorSize, &nBytesWritten);
				else
					throw exception("Unable to send request descriptor: buffer is null");
			}

			if (req.dataSize > 0) {
				if (req.data != nullptr)
					_dpipe->WriteRaw(req.data, req.dataSize, &nBytesWritten);
				else
					throw exception("Unable to send request data: buffer is null");
			}
		_mutexWrite.unlock();
		auto t1 = Time::now();

		WaitRequestLoop(requestRecord, timeout);

		auto t2 = Time::now();

		_requestListMutex.lock();
			_requestList.remove(requestRecord);
		_requestListMutex.unlock();

		if (requestRecord->succes) {
			shared_ptr<DPResponse> response = shared_ptr<DPResponse>(requestRecord->resp);
			response->_send_duraction = t1 - t0;
			response->_request_duraction = t2 - t1;

			delete requestRecord;
			return response;
		}
		else {
			delete requestRecord;
			return make_shared<DPResponse>(DP_REQUEST_TIMEOUT, 0, 0, false, nullptr, 0, 0, nullptr, _dpipe);
		}
	}

	shared_ptr<DPResponse> DPClient::SendRequest(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout)
	{
		DPRequest req;
		req.code = code;
		req.dataType = dataType;
		req.data = data;
		req.dataSize = dataSize;
		return SendRequest(req, timeout);
	}

	shared_ptr<DPResponse> DPClient::SendRequest(int code, void* data, DWORD dataSize, unsigned int timeout)
	{
		DPRequest req;
		req.code = code;
		req.dataType = DP_DATA_BINARY;
		req.data = data;
		req.dataSize = dataSize;
		return SendRequest(req, timeout);
	}

	void DPClient::SendRequestAsync(const DPRequest& req, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {
		
		DPRequestAsyncData* reqData = new DPRequestAsyncData();
		reqData->req = req;
		reqData->client = this;
		reqData->callback = receiveResponseCallback;
		reqData->timeout = timeout;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveResponseAsync, (LPVOID)reqData, 0, &_nReadThreadId);
	}

	void DPClient::SendRequestAsync(int code, int dataType, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

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

	void DPClient::SendRequestAsync(int code, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

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

	void DPClient::SetOnServerDisconnectCallback(function<void(IDPipe* pipe, PacketHeader header)> function) {
		_onServerDisconnect = function;
	}

	void DPClient::FreeAllocatedMemory() {
		HeapAllocatedData::Free();
	}

	void DPClient::FreeAllocatedMemory(void* data) {
		HeapAllocatedData::Free(data);
	}

	bool DPClient::Connect(IDPipeHandle* pHandle) {
		return _dpipe->Connect(pHandle);
	}

	bool DPClient::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) {
		return _dpipe->Connect(pHandle, pConnectData, nConnectDataSize);
	}

	bool DPClient::Connect(IDPipeHandle* pHandle, std::string message) {
		DWORD len = boost::numeric_cast<DWORD>(message.length());
		return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UTF8);
	}

	bool DPClient::Connect(IDPipeHandle* pHandle, std::wstring message) {
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UNICODE);
	}

	bool DPClient::Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) {
		return _dpipe->Connect(handleString, pConnectData, nConnectDataSize);
	}

	bool DPClient::Connect(const std::wstring handleString, std::string message) {
		DWORD len = boost::numeric_cast<DWORD>(message.length());
		return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UTF8);
	}

	bool DPClient::Connect(const std::wstring handleString, std::wstring message) {
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UNICODE);
	}

	bool DPClient::Connect(const std::wstring handleString) {
		return _dpipe->Connect(handleString);
	}

	void DPClient::Disconnect() {
		_dpipe->Disconnect();
	}

	void DPClient::Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize) {
		_dpipe->Disconnect(pConnectData, nConnectDataSize);
	}

#pragma endregion DPClient