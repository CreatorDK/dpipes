#include "dpserver.h"

using namespace std;
using namespace crdk::dpipes;

//DPReceivedRequest implementation
#pragma region DPSendingResponse
	DPSendingResponse::DPSendingResponse(GUID guid) {
		_guid = guid;
	}

	GUID DPSendingResponse::guid() const {
		return _guid;
	}
#pragma endregion DPSendingResponse

//DPReceivedRequest implementation
#pragma region DPReceivedRequest
	DPReceivedRequest::DPReceivedRequest(
		GUID guid,
		int code,
		int descriptorType,
		DWORD descriptorSize,
		bool descriptorAllocated,
		std::shared_ptr<HeapAllocatedData> descriptor,
		int dataType,
		DWORD dataSize,
		std::shared_ptr<HeapAllocatedData> data,
		IDPServer* server) {
		_guid = guid;
		_code = code;
		_descriptorType = descriptorType;
		_descriptorSize = descriptorSize;
		_descriptorAllocated = descriptorAllocated;
		_descriptor = descriptor;
		_dataType = dataType;
		_dataSize = dataSize;
		_data = data;
		_server = server;
	}

	DPReceivedRequest::~DPReceivedRequest() { }

	GUID DPReceivedRequest::guid() const {
		return _guid;
	}

	int DPReceivedRequest::code() const {
		return _code;
	}

	bool DPReceivedRequest::descriptorAllocated() const {
		return _descriptorAllocated;
	}

	int DPReceivedRequest::descriptorType() const {
		return _descriptorType;
	}

	DWORD DPReceivedRequest::descriptorSize() const {
		return _descriptorSize;
	}

	std::shared_ptr<HeapAllocatedData> DPReceivedRequest::descriptor() {
		return _descriptor;
	}

	void* DPReceivedRequest::descriptorPtr() const {
		return _descriptor->data();
	}

	bool DPReceivedRequest::dataAllocated() const {
		return _dataAllocated;
	}

	int DPReceivedRequest::dataType() const {
		return _dataType;
	}

	DWORD DPReceivedRequest::dataSize() const {
		return _dataSize;
	}

	shared_ptr<HeapAllocatedData> DPReceivedRequest::data() {
		return _data;
	}

	void* DPReceivedRequest::dataPtr() const {
		return _data->data();
	}

	bool DPReceivedRequest::keepDescriptor() {
		return _descriptor->keepAllocatedMemory;
	}

	void DPReceivedRequest::keepDescriptor(bool keepAllocatedMemory) {
		_descriptor->keepAllocatedMemory = keepAllocatedMemory;
	}

	bool DPReceivedRequest::keepData() {
		return _data->keepAllocatedMemory;
	}

	void DPReceivedRequest::keepData(bool keepAllocatedMemory) {
		_data->keepAllocatedMemory = keepAllocatedMemory;
	}

	void DPReceivedRequest::ReadDescriptor(bool keepAllocatedMemory) {

		if (!_descriptorAllocated && _descriptorSize > 0) {

			_descriptor.~shared_ptr();

			_descriptor = make_shared<HeapAllocatedData>(_descriptorSize, keepAllocatedMemory);
			_server->Pipe()->Read(_descriptor->data(), _descriptorSize);
			_descriptorAllocated = true;
		}

	}
	void DPReceivedRequest::Read(bool keepAllocatedMemory) {

		ReadDescriptor(keepAllocatedMemory);

		if (!_dataAllocated && _dataSize > 0) {

			_data.~shared_ptr();

			_data = make_shared<HeapAllocatedData>(_dataSize, keepAllocatedMemory);
			_server->Pipe()->Read(_data->data(), _dataSize);
			_dataAllocated = true;
		}
	}

	DPSendingResponse DPReceivedRequest::createRespose() {
		return DPSendingResponse(guid());
	}
	IDPServer* DPReceivedRequest::server() const {
		return _server;
	}

#pragma endregion DPReceivedRequest

//DPServer implementation
#pragma region DPServer

	DPServer::DPServer(IDPipe* dpipe, bool handleAsync) :
		IDPipeMessanger(dpipe)
	{
		if (_dpipe == nullptr)
			return;

		_handleRequestAsync = handleAsync;

		_nBufferRequestSize = DP_REQUEST_SIZE;
		_pBufferRequest = new char[DP_REQUEST_SIZE];

		_nBufferResponseSize = DP_RESPONSE_SIZE;
		_pBufferResponse = new char[DP_RESPONSE_SIZE];

		_dpipe->SetClientConnectCallback([=](IDPipe* pipe, PacketHeader header) { OnClientConnect(pipe, header); });
		_dpipe->SetOtherSideDisconnectCallback([=](IDPipe* pipe, PacketHeader header) { OnClientDisconnect(pipe, header); });
		_dpipe->SetPacketHeaderRecevicedCallback([=](IDPipe* pipe, PacketHeader header) { OnDataReceived(pipe, header); });
	}

	DPServer::~DPServer() { 
		_dpipe->SetClientConnectCallback({});
		_dpipe->SetOtherSideDisconnectCallback({});
		_dpipe->SetPacketHeaderRecevicedCallback({});

		delete[] _pBufferRequest;
		delete[] _pBufferResponse;
	}

	void DPServer::OnClientConnect(IDPipe* pipe, PacketHeader header) {
		if (_onClientConnect)
			_onClientConnect(pipe, header);
	}

	void DPServer::OnClientDisconnect(IDPipe* pipe, PacketHeader header) {
		if (_onClientDisconnect)
			_onClientDisconnect(pipe, header);
	}

	void DPServer::OnRequestReceived(IDPipe* pipe, DPRequestHeader& reqRecord, DPReceivedRequest req) {

		bool handlerFound = false;

		function<void(DPReceivedRequest&)> handlerFunction = nullptr;

		_handlerMutex.lock();
			for (DPHandler const &handler : _handlers) {
				if (handler.code == reqRecord.code) {
					handlerFound = true;
					handlerFunction = handler.function;
				}
			}
		_handlerMutex.unlock();

		if (handlerFunction != nullptr)
			handlerFunction(req);

		if (!handlerFound) {
			auto resp = req.createRespose();
			resp.code = DP_HANDLER_NOT_FOUND;
			req.keepDescriptor(false);
			req.keepData(false);
			SendResponse(req, resp);
		}
	}

	bool DPServer::keepDescriptorMemoryAllocated() {
		return _keepDescriptorMemoryAllocated;
	}

	void DPServer::keepDescriptorMemoryAllocated(bool value) {
		_keepDescriptorMemoryAllocated = value;
	}

	bool DPServer::keepRequestMemoryAllocated() {
		return _keepRequestMemoryAllocated;
	}

	void DPServer::keepRequestMemoryAllocated(bool value) {
		_keepRequestMemoryAllocated = value;
	}

	IDPipe* DPServer::Pipe() {
		return _dpipe;
	}

	DWORD DPServer::maxDescriptorMemoryAllocation() {
		return _maxDescriptorMemoryAllocation;
	}

	void DPServer::maxDescriptorMemoryAllocation(DWORD value) {
		_maxDescriptorMemoryAllocation = value;
	}

	void DPServer::maxRequestMemoryAllocation(DWORD value) {
		_maxRequestMemoryAllocation = value;
	}

	void DPServer::maxRequestMemoryAllocationMB(DWORD value) {
		_maxRequestMemoryAllocation = value * 1048576;
	}

	DWORD DPServer::maxRequestMemoryAllocation() {
		return _maxRequestMemoryAllocation;
	}

	DWORD DPServer::maxRequestMemoryAllocationMB() {
		return _maxRequestMemoryAllocation / 1048576;
	}

	void DPServer::OnRequestReceivedAsync(LPVOID dataPtr) {
		auto requestData = (DPRequestReceivedAsyncData*)dataPtr;
		requestData->server->OnRequestReceived(requestData->pipe, requestData->reqRecord, requestData->request);
		delete requestData;
	}

	void DPServer::OnDataReceived(IDPipe* pipe, PacketHeader header) {
		
		DWORD nBytesRead;
		//Get data code ignoring first 8 bits (using for encoding)
		DWORD dataCode = header.GetDataCodeOnly();

		//ReadData from Data Request
		if (dataCode == DP_REQUEST) {

			DWORD dataSize = header.DataSize();

			if (dataSize < DP_REQUEST_SIZE)
				throw exception("Received broken request");

			_dpipe->Read(_pBufferRequest, DP_REQUEST_SIZE, &nBytesRead, NULL);
			DPRequestHeader reqRecord;
			FillRequestRecord(reqRecord);

			bool descriptorAllocated;
			DWORD descriptorSize = reqRecord.descriptorSize;
			shared_ptr<HeapAllocatedData> descriptorData = nullptr;

			if (descriptorSize <= _maxDescriptorMemoryAllocation) {
				descriptorData = make_shared<HeapAllocatedData>(descriptorSize, _keepDescriptorMemoryAllocated);
				DWORD nBytesRead;
				_dpipe->Read(descriptorData->data(), descriptorSize, &nBytesRead);
				descriptorAllocated = true;
			}
			else {
				descriptorData = make_shared<HeapAllocatedData>(0, _keepDescriptorMemoryAllocated);
				descriptorAllocated = false;
			}

			bool dataAllocated;
			dataSize = dataSize - DP_RESPONSE_SIZE - descriptorSize;
			shared_ptr<HeapAllocatedData> data = nullptr;

			if (descriptorAllocated && dataSize <= _maxRequestMemoryAllocation) {
				data = make_shared<HeapAllocatedData>(dataSize, _keepRequestMemoryAllocated);
				DWORD nBytesRead;
				_dpipe->Read(data->data(), dataSize, &nBytesRead);
				dataAllocated = true;
			}
			else {
				data = make_shared<HeapAllocatedData>(0, _keepRequestMemoryAllocated);
				dataAllocated = false;
			}

			DPReceivedRequest req(
				reqRecord.guid,
				reqRecord.code,
				reqRecord.descriptorType,
				reqRecord.descriptorSize,
				descriptorAllocated,
				descriptorData,
				reqRecord.dataType,
				dataSize,
				data,
				this
			);

			if (_handleRequestAsync) {
				DWORD _nReadThreadId;
				DPRequestReceivedAsyncData* reqData = new DPRequestReceivedAsyncData(this, reqRecord, req, _dpipe);
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRequestReceivedAsync, (LPVOID)reqData, 0, &_nReadThreadId);
			}
			else
				OnRequestReceived(pipe, reqRecord, req);

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

			if (_handleRequestAsync) {
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

			if (_handleRequestAsync) {
				DPMessageReceivedAsyncData* messageData = new DPMessageReceivedAsyncData(this, header, heapData, pipe);

				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
			}
			else
				OnMessageReceived(pipe, header, heapData);
		}
	}

	void DPServer::FillRequestRecord(const DPRequestHeader& reqRecord) const {
		memcpy_s((char*)&reqRecord.guid, sizeof(GUID), _pBufferRequest, sizeof(GUID));
		memcpy_s((char*)&reqRecord.code, sizeof(int), _pBufferRequest + sizeof(GUID), sizeof(int));
		memcpy_s((char*)&reqRecord.dataType, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int));
		memcpy_s((char*)&reqRecord.descriptorType, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int) * 2, sizeof(int));
		memcpy_s((char*)&reqRecord.descriptorSize, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int) * 3, sizeof(int));
	}

	void DPServer::PrepareResponce(const DPSendingResponse& response, const GUID& guid) {
		memcpy_s(_pBufferResponse, sizeof(GUID), (char*)&guid, sizeof(GUID));
		memcpy_s(_pBufferResponse + sizeof(GUID), sizeof(int), (char*)&response.code, sizeof(int));
		memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&response.dataType, sizeof(int));
		memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int) * 2, sizeof(int), (char*)&response.descriptorType, sizeof(int));
		memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int) * 3, sizeof(int), (char*)&response.descriptorSize, sizeof(int));
	}

	void DPServer::SendResponse(const DPReceivedRequest& req, const DPSendingResponse& resp)
	{
		_mutexWrite.lock();
			PrepareResponce(resp, req.guid());
			DWORD nBytesWritten;
			DWORD dataLen = _nBufferResponseSize + resp.dataSize;
			PacketHeader header(false, dataLen);
			header.SetDataCode(DP_RESPONSE);

			_dpipe->WritePacketHeader(header);
			_dpipe->WriteRaw(_pBufferResponse, DP_RESPONSE_SIZE, &nBytesWritten);

			if (resp.dataSize > 0) {
				if (resp.data != nullptr)
					_dpipe->WriteRaw(resp.data, resp.dataSize, &nBytesWritten);
				else
					throw exception("Unable to send response data: buffer is null");
			}
		_mutexWrite.unlock();
	}

	void DPServer::SetHandler(int code, function<void(DPReceivedRequest&)> function) {
		DPHandler handler;
		handler.code = code;
		handler.function = function;

		_handlerMutex.lock();
			_handlers.push_back(handler);
		_handlerMutex.unlock();
	}

	void DPServer::RemoveHandler(int code) {
		_handlerMutex.lock();
		for (auto it = _handlers.begin(); it != _handlers.end(); it++) {
			if (it->code == code) {
				it->function = nullptr;
				_handlers.erase(it);
				break;
			}
		}
		_handlerMutex.unlock();
	}

	void DPServer::SetOnClientConnectCallback(function<void(IDPipe* pipe, PacketHeader header)> function) {
		_onClientConnect = function;
	}
	
	void DPServer::SetOnClientDisconnectCallback(function<void(IDPipe* pipe, PacketHeader header)> function) {
		_onClientDisconnect = function;
	}

	void DPServer::FreeAllocatedMemory(void* data) {
		HeapAllocatedData::Free(data);
	}

	void DPServer::FreeAllocatedMemory() {
		HeapAllocatedData::Free();
	}

	void DPServer::Disconnect() {
		_dpipe->Disconnect();
	}

	void DPServer::Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize) {
		_dpipe->Disconnect(pConnectData, nConnectDataSize);
	}

	bool DPServer::Start() {
		return _dpipe->Start();
	}

	shared_ptr<IDPipeHandle> DPServer::GetHandle() {
		return _dpipe->GetHandle();
	}
#pragma endregion DPServer

	DPRequestReceivedAsyncData::DPRequestReceivedAsyncData(
		IDPServer* serverPtr,
		DPRequestHeader reqRec, 
		DPReceivedRequest clientRequest, 
		IDPipe* pipePtr) :
		request(clientRequest)
	{
		server = serverPtr;
		reqRecord = reqRec;
		pipe = pipePtr;
	}
