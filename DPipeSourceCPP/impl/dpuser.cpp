#include "dpuser.h"

using namespace std;
using namespace crdk::dpipes;

typedef std::chrono::high_resolution_clock Time;

//DPUser implementation
#pragma region DPUser
DPUser::DPUser(IDPipe* dpipe, bool handleAsync) :
	IDPipeMessanger(dpipe)
{
	if (_dpipe == nullptr)
		return;

	_handleAsync = handleAsync;

	_nBufferRequestSize = DP_REQUEST_SIZE;
	_pBufferRequest = new char[DP_REQUEST_SIZE];

	_nBufferResponseSize = DP_RESPONSE_SIZE;
	_pBufferResponse = new char[DP_RESPONSE_SIZE];

	_dpipe->SetPacketHeaderRecevicedCallback([=](IDPipe* pipe, PacketHeader header) { OnDataReceived(pipe, header); });
	_dpipe->SetOtherSideDisconnectCallback([=](IDPipe* pipe, PacketHeader header) { OnOtherSideDisconnecting(pipe, header); });
	_dpipe->SetPacketHeaderRecevicedCallback([=](IDPipe* pipe, PacketHeader header) { OnDataReceived(pipe, header); });
}

DPUser::~DPUser()
{
	_dpipe->SetPacketHeaderRecevicedCallback({});
	_dpipe->SetOtherSideDisconnectCallback({});
	_dpipe->SetPacketHeaderRecevicedCallback({});

	delete[] _pBufferRequest;
	delete[] _pBufferResponse;
}

void DPUser::OnOtherSideConnecting(IDPipe* pipe, PacketHeader header) {
	if (_onOtherSideConnecting)
		_onOtherSideConnecting(pipe, header);
}

void DPUser::OnOtherSideDisconnecting(IDPipe* pipe, PacketHeader header) {
	if (_onOtherSideDisconnecting)
		_onOtherSideDisconnecting(pipe, header);
}

void DPUser::OnDataReceived(IDPipe* pipe, PacketHeader header) {

	DWORD nBytesRead;
	//Get data code ignoring first 8 bits (using for encoding)
	DWORD dataCode = header.GetDataCodeOnly();

	//ReadData from Response
	if (dataCode == DP_RESPONSE) {
		OnResponseReceived(header);
		return;
	}

	//ReadData from Request
	else if (dataCode == DP_REQUEST) {

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

		if (descriptorAllocated && dataSize <= _maxRequestMemoryAllocation) {
			data = make_shared<HeapAllocatedData>(dataSize, _keepRequestMemoryAllocated);
			_dpipe->Read(data->data(), dataSize);
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

		if (_handleAsync) {
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

		if (_handleAsync) {
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

		if (_handleAsync) {
			DPMessageReceivedAsyncData* messageData = new DPMessageReceivedAsyncData(this, header, heapData, pipe);
			DWORD _nReadThreadId;
			auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageReceivedAsync, (LPVOID)messageData, 0, &_nReadThreadId);
		}
		else
			OnMessageReceived(pipe, header, heapData);
	}
}

void DPUser::OnResponseReceived(PacketHeader& header) {

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
		data = make_shared<HeapAllocatedData>(0, _keepResponseMemoryAllocated);
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

void DPUser::SetHandler(int code, function<void(DPReceivedRequest&)> function) {
	DPHandler handler;
	handler.code = code;
	handler.function = function;

	_handlerMutex.lock();
	_handlers.push_back(handler);
	_handlerMutex.unlock();
}

void DPUser::RemoveHandler(int code) {
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

void DPUser::SendResponse(const DPReceivedRequest& req, const DPSendingResponse& resp) {
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

void DPUser::OnRequestReceived(IDPipe* pipe, DPRequestHeader& reqRecord, DPReceivedRequest req) {

	bool handlerFound = false;

	function<void(DPReceivedRequest&)> handlerFunction = nullptr;

	_handlerMutex.lock();
	for (DPHandler const& handler : _handlers) {
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

void DPUser::OnRequestReceivedAsync(LPVOID dataPtr) {
	auto requestData = (DPRequestReceivedAsyncData*)dataPtr;
	requestData->server->OnRequestReceived(requestData->pipe, requestData->reqRecord, requestData->request);
	delete requestData;
}

void DPUser::WaitRequestLoop(DPRequestRecord* requsertRecord, unsigned int timeout) {

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

void DPUser::FillResponseHeader(const DPResponseHeader& responseRecord) const {
	memcpy_s((char*)&responseRecord.guid, sizeof(GUID), _pBufferResponse, sizeof(GUID));
	memcpy_s((char*)&responseRecord.code, sizeof(int), _pBufferResponse + sizeof(GUID), sizeof(int));
	memcpy_s((char*)&responseRecord.dataType, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int), sizeof(int));
	memcpy_s((char*)&responseRecord.descriptorType, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int) * 2, sizeof(int));
	memcpy_s((char*)&responseRecord.descriptorSize, sizeof(int), _pBufferResponse + sizeof(GUID) + sizeof(int) * 3, sizeof(int));
}

IDPipe* DPUser::Pipe() {
	return _dpipe;
}

void DPUser::PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord) {
	memcpy_s(_pBufferRequest, sizeof(GUID), (char*)&requestRecord->guid, sizeof(GUID));
	memcpy_s(_pBufferRequest + sizeof(GUID), sizeof(int), (char*)&request.code, sizeof(int));
	memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&request.dataType, sizeof(int));
	memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int) * 2, sizeof(int), (char*)&request.descriptorType, sizeof(int));
	memcpy_s(_pBufferRequest + sizeof(GUID) + sizeof(int) * 3, sizeof(int), (char*)&request.descriptorSize, sizeof(int));
}

shared_mutex* DPUser::GetWriteMutex(){
	return &_mutexWrite;
}

char* DPUser::RequestBuffer() {
	return _pBufferRequest;
}

char* DPUser::ResponseBuffer() {
	return _pBufferResponse;
}

bool DPUser::keepDescriptorMemoryAllocated() {
	return _keepDescriptorMemoryAllocated;
}

void DPUser::keepDescriptorMemoryAllocated(bool value) {
	_keepDescriptorMemoryAllocated = value;
}

bool DPUser::keepResponseMemoryAllocated() {
	return _keepResponseMemoryAllocated;
}

void DPUser::keepResponseMemoryAllocated(bool value) {
	_keepResponseMemoryAllocated = value;
}

DWORD DPUser::maxDescriptorMemoryAllocation() {
	return _maxDescriptorMemoryAllocation;
}

void DPUser::maxDescriptorMemoryAllocation(DWORD value) {
	_maxDescriptorMemoryAllocation = value;
}

DWORD DPUser::maxResponseMemoryAllocation() {
	return _maxResponseMemoryAllocation;
}

DWORD DPUser::maxResponseMemoryAllocationMB() {
	return _maxResponseMemoryAllocation / 1048576;
}

void DPUser::maxResponseMemoryAllocation(DWORD value) {
	_maxResponseMemoryAllocation = value;
}

void DPUser::maxResponseMemoryAllocationMB(DWORD value) {
	_maxResponseMemoryAllocation = value * 1048576;
}

bool DPUser::keepRequestMemoryAllocated() {
	return _keepRequestMemoryAllocated;
}

void DPUser::keepRequestMemoryAllocated(bool value) {
	_keepRequestMemoryAllocated = value;
}

DWORD DPUser::maxRequestMemoryAllocation() {
	return _maxRequestMemoryAllocation;
}

DWORD DPUser::maxRequestMemoryAllocationMB() {
	return _maxRequestMemoryAllocation / 1048576;
}

void DPUser::maxRequestMemoryAllocation(DWORD value) { 
	_maxRequestMemoryAllocation = value;
}

void DPUser::maxRequestMemoryAllocationMB(DWORD value) {
	_maxRequestMemoryAllocation = value * 1048576;
}

void DPUser::FillRequestRecord(const DPRequestHeader& reqRecord) const {
	memcpy_s((char*)&reqRecord.guid, sizeof(GUID), _pBufferRequest, sizeof(GUID));
	memcpy_s((char*)&reqRecord.code, sizeof(int), _pBufferRequest + sizeof(GUID), sizeof(int));
	memcpy_s((char*)&reqRecord.dataType, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int), sizeof(int));
	memcpy_s((char*)&reqRecord.descriptorType, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int) * 2, sizeof(int));
	memcpy_s((char*)&reqRecord.descriptorSize, sizeof(int), _pBufferRequest + sizeof(GUID) + sizeof(int) * 3, sizeof(int));
}

void DPUser::PrepareResponce(const DPSendingResponse& response, const GUID& guid) {
	memcpy_s(_pBufferResponse, sizeof(GUID), (char*)&guid, sizeof(GUID));
	memcpy_s(_pBufferResponse + sizeof(GUID), sizeof(int), (char*)&response.code, sizeof(int));
	memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int), sizeof(int), (char*)&response.dataType, sizeof(int));
	memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int) * 2, sizeof(int), (char*)&response.descriptorType, sizeof(int));
	memcpy_s(_pBufferResponse + sizeof(GUID) + sizeof(int) * 3, sizeof(int), (char*)&response.descriptorSize, sizeof(int));
}

void DPUser::ReceiveResponseAsync(LPVOID dataPtr) {

	DPRequestAsyncData* reqData = (DPRequestAsyncData*)dataPtr;
	IDPClient* client = reqData->client;

	auto resp = client->SendRequest(reqData->req, reqData->timeout);

	if (reqData->callback)
		reqData->callback(resp);

	delete reqData;
}

shared_ptr<DPResponse> DPUser::SendRequest(const DPRequest& req, unsigned int timeout)
{
	typedef chrono::high_resolution_clock Time;
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

shared_ptr<DPResponse> DPUser::SendRequest(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout)
{
	DPRequest req;
	req.code = code;
	req.dataType = dataType;
	req.data = data;
	req.dataSize = dataSize;
	return SendRequest(req, timeout);
}

shared_ptr<DPResponse> DPUser::SendRequest(int code, void* data, DWORD dataSize, unsigned int timeout)
{
	DPRequest req;
	req.code = code;
	req.dataType = DP_DATA_BINARY;
	req.data = data;
	req.dataSize = dataSize;
	return SendRequest(req, timeout);
}

void DPUser::SendRequestAsync(const DPRequest& req, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

	DPRequestAsyncData* reqData = new DPRequestAsyncData();
	reqData->req = req;
	reqData->client = this;
	reqData->callback = receiveResponseCallback;
	reqData->timeout = timeout;

	DWORD _nReadThreadId;
	auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveResponseAsync, (LPVOID)reqData, 0, &_nReadThreadId);
}

void DPUser::SendRequestAsync(int code, int dataType, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

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

void DPUser::SendRequestAsync(int code, void* data, DWORD dataSize, function<void(shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout) {

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

void DPUser::SetOnOtherSideConnectCallback(function<void(IDPipe* pipe, PacketHeader header)> function) {
	_onOtherSideConnecting = function;
}

void crdk::dpipes::DPUser::SetOnOtherSideDisconnectCallback(function<void(IDPipe* pipe, PacketHeader header)> function) {
	_onOtherSideDisconnecting = function;
}

void DPUser::FreeAllocatedMemory() {
	HeapAllocatedData::Free();
}

void DPUser::FreeAllocatedMemory(void* data) {
	HeapAllocatedData::Free(data);
}

void crdk::dpipes::DPUser::FreeAllocatedMemory(unsigned long long dataAddress) {
	HeapAllocatedData::Free(dataAddress);
}

bool DPUser::Connect(IDPipeHandle* pHandle) {
	return _dpipe->Connect(pHandle);
}

bool DPUser::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) {
	return _dpipe->Connect(pHandle, pConnectData, nConnectDataSize);
}

bool DPUser::Connect(IDPipeHandle* pHandle, string message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length());
	return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UTF8);
}

bool DPUser::Connect(IDPipeHandle* pHandle, wstring message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
	return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UNICODE);
}

bool DPUser::Connect(const wstring handleString) {
	return _dpipe->Connect(handleString);
}

bool DPUser::Connect(const wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) {
	return _dpipe->Connect(handleString, pConnectData, nConnectDataSize);
}

bool DPUser::Connect(const wstring handleString, string message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length());
	return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UTF8);
}

bool DPUser::Connect(const wstring handleString, wstring message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
	return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UNICODE);
}

void DPUser::Disconnect() {
	_dpipe->Disconnect();
}

void DPUser::Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize) {
	_dpipe->Disconnect(pConnectData, nConnectDataSize);
}

#pragma endregion DPUser

void DPUser::SetMemoryAllocatedCallback(function<void(void* address, DWORD size)> function) {
	HeapAllocatedData::SetMemoryAllocatedCallback(function);
}

void DPUser::SetMemoryDeallocatedCallback(function<void(void* address, DWORD size)> function) {
	HeapAllocatedData::SetMemoryDeallocatedCallback(function);
}

void DPUser::ReadAllocatedAddresses(list<pair<void*, DWORD>>& list) {
	return HeapAllocatedData::ReadAllocatedAddresses(list);
}

function<void(void* address, DWORD size)> DPUser::_onMemoryAllocatedCallback;
function<void(void* address, DWORD size)> DPUser::_onMemoryDeallocatedCallback;

void DPFileTransporter::OnFileRequestReceived(DPReceivedRequest& request) {

	if (_onFileReceiveRequest) {
		auto encodingCode = request.descriptorType();

		if (!request.descriptorAllocated()) {
			request.ReadDescriptor();
		}

		DPFileReceiverRequest fileReceiveRequest(
			request.descriptor(), request.descriptorType(), request.data(), request.dataSize(), request.dataAllocated(), request.server()
		);

		_onFileReceiveRequest(fileReceiveRequest);
	}
}

DPFileTransporter::DPFileTransporter(DPUser* dpuser, int code) {
	_dpuser = dpuser;
	_code = code;
	_dpuser->SetHandler(_code, [=](DPReceivedRequest& request) { OnFileRequestReceived(request); });
}

DPFileTransporter::~DPFileTransporter() {
	_dpuser->RemoveHandler(_code);
}

void DPFileTransporter::SetFileReceivedCallback(std::function<void(DPFileReceiverRequest&)> function) {
	_onFileReceiveRequest = function;
}

void DPFileTransporter::SendFile(std::ifstream& filestream, void* descriptor, DWORD descriptorSize, DWORD encodingCode, DWORD bufferSize) {

	filestream.seekg(0, std::ios::end);
	std::streamsize fileSizeStream = filestream.tellg();
	filestream.seekg(0, std::ios::beg);

	DPRequest req;
	req.code = _code;
	req.dataType = 0;
	req.dataSize = boost::numeric_cast<DWORD>(fileSizeStream);
	req.descriptorType = encodingCode;
	req.descriptorSize = descriptorSize;

	DPRequestRecord* requestRecord = new DPRequestRecord();
	auto res = CoCreateGuid(&requestRecord->guid);

	_dpuser->PrepareRequestRecord(req, requestRecord);

	DWORD packetSize = DP_REQUEST_SIZE + req.descriptorSize + req.dataSize;

	PacketHeader header(false, packetSize);
	header.SetDataCode(DP_REQUEST);

	auto writeMutex = _dpuser->GetWriteMutex();
	auto pipe = _dpuser->Pipe();

	writeMutex->lock();

		pipe->WritePacketHeader(header);
		pipe->WriteRaw(_dpuser->RequestBuffer(), DP_REQUEST_SIZE);

		if (descriptorSize > 0)
			pipe->WriteRaw(descriptor, descriptorSize);

		vector<char> buffer(bufferSize);

		auto cycles = req.dataSize / bufferSize;
		auto rest = req.dataSize % bufferSize;

		for (DWORD i = 0; i < cycles; i++) {
			filestream.read(buffer.data(), bufferSize);
			pipe->WriteRaw(buffer.data(), bufferSize);
		}

		filestream.read(buffer.data(), rest);
		pipe->WriteRaw(buffer.data(), rest);

	writeMutex->unlock();
}

void DPFileTransporter::SendFile(std::ifstream& filestream, const std::string& fileName, DWORD bufferSize) {
	auto encodingCode = IDPipeMessanger::GetEncodingCode(DP_STRING_TYPE::STRING);
	auto descriptorSize = boost::numeric_cast<DWORD>(fileName.length());
	SendFile(filestream, (void*)fileName.c_str(), descriptorSize, encodingCode, bufferSize);
}

void DPFileTransporter::SendFile(std::ifstream& filestream, const std::wstring& fileName, DWORD bufferSize) {
	auto encodingCode = IDPipeMessanger::GetEncodingCode(DP_STRING_TYPE::WSTRING);
	auto descriptorSize = boost::numeric_cast<DWORD>(fileName.length() * sizeof(wchar_t));
	SendFile(filestream, (void*)fileName.c_str(), descriptorSize, encodingCode, bufferSize);
}

DPFileReceiverRequest::DPFileReceiverRequest(
	std::shared_ptr<HeapAllocatedData> fileNameData, 
	DWORD encodingCode, 
	std::shared_ptr<HeapAllocatedData> fileData, 
	DWORD fileSize, 
	bool fileDataAllocated,
	IDPServer* server
	) {

	_fileNameData = fileNameData;
	_encodingCode = encodingCode;
	_fileData = fileData;
	_fileSize = fileSize;
	_fileDataAllocated = fileDataAllocated;
	_server = server;
}

string DPFileReceiverRequest::GetFileName() {
	return IDPipeMessanger::GetString(_fileNameData, _encodingCode);
}

wstring DPFileReceiverRequest::GetFileNameW() {
	return IDPipeMessanger::GetWString(_fileNameData, _encodingCode);
}

DWORD DPFileReceiverRequest::GetFileSize() {
	return _fileSize;
}

void DPFileReceiverRequest::SaveFile(std::ofstream& fs, DWORD bufferSize) {

	if (_fileDataAllocated) {
		fs.write((char*)_fileData->data(), _fileData->size());
	}
	else {

		auto cycles = _fileSize / bufferSize;
		auto rest = _fileSize % bufferSize;

		vector<char> buffer = vector<char>(bufferSize);
		auto pipe = _server->Pipe();

		for (DWORD i = 0; i < cycles; i++) {
			pipe->Read(buffer.data(), bufferSize);
			fs.write(buffer.data(), bufferSize);
		}

		pipe->Read(buffer.data(), rest);
		fs.write(buffer.data(), rest);
	}
}
