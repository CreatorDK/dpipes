#include "dpserver.h"

using namespace std;
using namespace crdk::dpipes;

//DPClientRequest implementation
#pragma region DPClientResponse
	DPClientResponse::DPClientResponse(GUID guid) {
		_guid = guid;
	}

	GUID DPClientResponse::guid() const {
		return _guid;
	}
#pragma endregion DPClientResponse

//DPClientRequest implementation
#pragma region DPClientRequest
	DPClientRequest::DPClientRequest(GUID guid, int code, int dataType, shared_ptr<HeapAllocatedData> data) {
		_guid = guid;
		_code = code;
		_dataType = dataType;
		_data = data;
	}

	DPClientRequest::~DPClientRequest() { }

	GUID DPClientRequest::guid() const {
		return _guid;
	}

	int DPClientRequest::code() const {
		return _code;
	}

	int DPClientRequest::dataType() const {
		return _dataType;
	}

	void* DPClientRequest::data() const {
		return _data->data();
	}

	std::shared_ptr<HeapAllocatedData> DPClientRequest::getData() {
		return _data;
	}

	DWORD DPClientRequest::dataSize() const {
		return _data->size();
	}

	void DPClientRequest::keepAllocatedMemory(bool keepAllocatedMemory){
		_data->keepAllocatedMemory = keepAllocatedMemory;
	}

	bool DPClientRequest::keepAllocatedMemory() {
		return _data->keepAllocatedMemory;
	}

	DPClientResponse DPClientRequest::createRespose() {
		return DPClientResponse(guid());
	}

#pragma endregion DPClientRequest

//DPServer implementation
#pragma region DPServer

	DPServer::DPServer(IDPipe* dpipe, bool handleAsync, DWORD nBufferStringSize) :
		IDPipeMessanger(dpipe, nBufferStringSize)
	{
		if (_dpipe == nullptr)
			return;

		_handleAsync = handleAsync;

		_nBufferRequestSize = DP_REQUEST_SIZE;
		_pBufferRequest = new char[DP_REQUEST_SIZE];

		_nBufferResponseSize = DP_RESPONSE_SIZE;
		_pBufferResponse = new char[DP_RESPONSE_SIZE];

		_dpipe->SetClientConnectCallback([=](PacketHeader header) { OnClientConnect(header); });
		_dpipe->SetOtherSideDisconnectCallback([=](PacketHeader header) { OnClientDisconnect(header); });
		_dpipe->SetPacketHeaderRecevicedCallback([=](PacketHeader header) { OnDataReceived(header); });
	}

	DPServer::~DPServer() { 
		_dpipe->SetClientConnectCallback(nullptr);
		_dpipe->SetOtherSideDisconnectCallback(nullptr);
		_dpipe->SetPacketHeaderRecevicedCallback(nullptr);

		delete[] _pBufferRequest;
		delete[] _pBufferResponse;
	}

	void DPServer::OnClientConnect(PacketHeader header) {
		DWORD dataSize = header.DataSize();
		DWORD nBytesRead;

		//_dpipe->Read(_pBufferRequest, DP_REQUEST_SIZE, &nBytesRead, NULL);
		auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

		if (dataSize > 0) {
			void* buffer = heapData->data();
			_dpipe->Read(buffer, dataSize, &nBytesRead, NULL);
		}

		if (_onClientConnect)
			_onClientConnect(heapData);
	}

	void DPServer::OnClientDisconnect(PacketHeader header) {
		DWORD dataSize = header.DataSize();
		DWORD nBytesRead;

		//_dpipe->Read(_pBufferRequest, DP_REQUEST_SIZE, &nBytesRead, NULL);
		auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

		if (dataSize > 0) {
			void* buffer = heapData->data();
			_dpipe->Read(buffer, dataSize, &nBytesRead, NULL);
		}

		if (_onClientDisconnect)
			_onClientDisconnect(heapData);
	}

	void DPServer::OnRequestReceived(DPRequestHeader& reqRecord, shared_ptr<HeapAllocatedData> data) {

		DPClientRequest req(reqRecord.guid, reqRecord.code, reqRecord.dataType, data);
		bool handlerFound = false;

		function<void(DPClientRequest&)> handlerFunction = nullptr;

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
			req.keepAllocatedMemory(false);
			SendResponse(req, resp);
		}
	}

	void DPServer::OnRequestReceivedAsync(LPVOID dataPtr) {
		auto requestData = (DPRequestReceivedAsyncData*)dataPtr;
		auto server = requestData->serverPtr;
		server->OnRequestReceived(requestData->reqRecord, requestData->data);
		delete requestData;
	}

	void DPServer::OnDataReceived(PacketHeader header) {
		
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
			//ReadData from Data Request
			if (serviceCode == 0) {
				DWORD dataSize = header.DataSize();

				if (dataSize < DP_REQUEST_SIZE)
					throw exception("Received broken request");

				//_mutexReadPipe.lock();

					_dpipe->Read(_pBufferRequest, DP_REQUEST_SIZE, &nBytesRead, NULL);
					DPRequestHeader reqRecord;
					FillRequestRecord(reqRecord);

					dataSize = dataSize - DP_REQUEST_SIZE;
					auto heapData = make_shared<HeapAllocatedData>(dataSize, keepRequestAllocatedMemory);

					if (dataSize > 0) {
						void* buffer = heapData->data();
						_dpipe->Read(buffer, dataSize, &nBytesRead, NULL);
					}

				//_mutexReadPipe.unlock();

				if (_handleAsync) {
					DPRequestReceivedAsyncData* reqData = new DPRequestReceivedAsyncData;
					reqData->serverPtr = this;
					reqData->reqRecord = reqRecord;
					reqData->data = heapData;

					DWORD _nReadThreadId;
					auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRequestReceivedAsync, (LPVOID)reqData, 0, &_nReadThreadId);
				}
				else
					OnRequestReceived(reqRecord, heapData);
			}

			//ReadData from Data Message
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

	void DPServer::FillRequestRecord(const DPRequestHeader& reqRecord) const {
		memcpy_s((char*)&reqRecord.guid, sizeof(GUID), _pBufferRequest, sizeof(GUID));
		memcpy_s((char*)&reqRecord.code, sizeof(int), _pBufferRequest + sizeof(GUID), sizeof(int));
		memcpy_s((char*)&reqRecord.dataType, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int));
	}

	void DPServer::PrepareResponce(const DPClientResponse& response, const GUID& guid) {
		memcpy_s(_pBufferResponse, sizeof(GUID), (char*)&guid, sizeof(GUID));
		memcpy_s(_pBufferResponse + sizeof(GUID), sizeof(int), (char*)&response.code, sizeof(int));
		memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&response.dataType, sizeof(int));
	}

	void DPServer::SendResponse(const DPClientRequest& req, const DPClientResponse& resp)
	{
		_mutexWritePipe.lock();
			PrepareResponce(resp, req.guid());
			DWORD nBytesWritten;
			DWORD dataLen = _nBufferResponseSize + resp.dataSize;
			PacketHeader header(dataLen, DP_RESPONSE);
			_dpipe->WritePacketHeader(header);
			_dpipe->WriteRaw(_pBufferResponse, DP_RESPONSE_SIZE, &nBytesWritten);

			if (resp.dataSize > 0) {
				if (resp.data != nullptr)
					_dpipe->WriteRaw(resp.data, resp.dataSize, &nBytesWritten);
				else
					throw exception("Unable to send response data: buffer is null");
			}
		_mutexWritePipe.unlock();

	}

	void DPServer::SetHandler(int code, function<void(DPClientRequest&)> function) {
		DPHandler handler;
		handler.code = code;
		handler.function = function;

		_handlerMutex.lock();
			_handlers.push_back(handler);
		_handlerMutex.unlock();
	}

	void DPServer::SetOnClientConnectCallback(function<void(std::shared_ptr<HeapAllocatedData> heapData)> function) {
		_onClientConnect = function;
	}
	
	void DPServer::SetOnClientDisconnectCallback(function<void(std::shared_ptr<HeapAllocatedData> heapData)> function) {
		_onClientDisconnect = function;
	}

	void DPServer::FreeAllocatedMemory(void* data) {
		delete[] data;
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