#include "dpmessangerbase.h"

using namespace std;
using namespace crdk::dpipes;

DPMessageReceivedAsyncData::DPMessageReceivedAsyncData(
	IDPipeMessanger* messangerPtr,
	PacketHeader packetHeader,
	std::shared_ptr<HeapAllocatedData> heapData,
	IDPipe* dpipe) :
	header(false, packetHeader.DataSize()) {
	header.SetDataCode(packetHeader.GetDataCode());
	messanger = messangerPtr;
	data = heapData;
	pipe = dpipe;
}

SendDataAsyncContainer::SendDataAsyncContainer(
	IDPipeMessanger* messangerPtr, 
	DP_MESSAGE_TYPE messageType, 
	void* dataPtr, 
	DWORD dataSize, 
	std::function<void(IDPipe*)> callbackFunction) {
	dpMessanger = messangerPtr;
	type = messageType;
	data = dataPtr;
	size = dataSize;
	callback = callbackFunction;
}

SendStringAsyncContainer::SendStringAsyncContainer(
	IDPipeMessanger* messangerPtr,
	DP_MESSAGE_TYPE messageType,
	string dataStr,
	function<void(IDPipe*)> callbackFunction) {
	dpMessanger = messangerPtr;
	type = messageType;
	data = dataStr;
	callback = callbackFunction;
}

SendWStringAsyncContainer::SendWStringAsyncContainer(
	IDPipeMessanger* messangerPtr,
	DP_MESSAGE_TYPE messageType,
	wstring dataStr,
	function<void(IDPipe*)> callbackFunction) {
	dpMessanger = messangerPtr;
	type = messageType;
	data = dataStr;
	callback = callbackFunction;
}


IDPipeMessanger::IDPipeMessanger(IDPipe* dpipe) {
	_dpipe = dpipe;
}

IDPipeMessanger::~IDPipeMessanger() {

}

void IDPipeMessanger::OnMessageReceived(IDPipe* pipe, PacketHeader& header, std::shared_ptr<HeapAllocatedData> heapData) {

	switch (header.GetDataCodeOnly())
	{
	case DP_MESSAGE_DATA:
		if (_onMessageDataReceived)
			_onMessageDataReceived(pipe, header, heapData);
		break;
	case DP_INFO_DATA:
		if (_onInfoDataReceived)
			_onInfoDataReceived(pipe, header, heapData);
		break;
	case DP_WARNING_DATA:
		if (_onWarningDataReceived)
			_onWarningDataReceived(pipe, header, heapData);
		break;
	case DP_ERROR_DATA:
		if (_onErrorDataReceived)
			_onErrorDataReceived(pipe, header, heapData);
		break;
	}
}

void IDPipeMessanger::OnMessageReceivedAsync(LPVOID dataPtr) {
	auto messageData = (DPMessageReceivedAsyncData*)dataPtr;
	IDPipeMessanger* messanger = messageData->messanger;
	messanger->OnMessageReceived(messageData->pipe, messageData->header, messageData->data);
	delete messageData;
}

void IDPipeMessanger::OnMessageStringReceived(IDPipe* pipe, PacketHeader& header, std::shared_ptr<HeapAllocatedData> heapData) {
	
	//Get data code ignoring first 8 bits (using for encoding)
	switch (header.GetDataCodeOnly())
	{
	case DP_MESSAGE_STRING:
		if (_onMessageStringReceived)
			_onMessageStringReceived(pipe, header, heapData);
		break;
	case DP_INFO_STRING:
		if (_onInfoStringReceived)
			_onInfoStringReceived(pipe, header, heapData);
		break;
	case DP_WARNING_STRING:
		if (_onWarningStringReceived)
			_onWarningStringReceived(pipe, header, heapData);
		break;
	case DP_ERROR_STRING:
		if (_onErrorStringReceived)
			_onErrorStringReceived(pipe, header, heapData);
		break;
	}
}

void IDPipeMessanger::OnMessageStringReceivedAsync(LPVOID dataPtr) {
	auto messageData = (DPMessageReceivedAsyncData*)dataPtr;
	IDPipeMessanger* messanger = messageData->messanger;
	messanger->OnMessageStringReceived(messageData->pipe, messageData->header, messageData->data);
	delete messageData;
}

DWORD IDPipeMessanger::GetEncodingCode(PacketHeader header) {
	if (header.IsService())
		return header.GetServicePrefix();

	return header.GetDataPrefix();
}

DWORD IDPipeMessanger::GetEncodingCode(DP_STRING_TYPE stringType) {
	if (stringType == DP_STRING_TYPE::STRING)
		return DP_ENCODING_UTF8;
	else if (stringType == DP_STRING_TYPE::WSTRING)
		return DP_ENCODING_UNICODE;
	return DP_ENCODING_UTF8;
}

DWORD IDPipeMessanger::AddEncodingCode(DWORD serviceCode, DWORD encodingCode) {
	auto encodingRaw = encodingCode << 24;
	return serviceCode | encodingRaw;
}

DP_STRING_TYPE IDPipeMessanger::GetStringType(DWORD encodingCode)
{
	if (encodingCode == DP_ENCODING_UTF8)
		return DP_STRING_TYPE::STRING;
	else if (encodingCode == DP_ENCODING_UNICODE)
		return DP_STRING_TYPE::WSTRING;
	else
		return DP_STRING_TYPE::UNDEFINED;
}

DP_STRING_TYPE IDPipeMessanger::GetStringType(PacketHeader header) {
	return GetStringType(GetEncodingCode(header));
}

wstring IDPipeMessanger::StringToWString(const std::string& str) {
	wstring wstr;
	size_t size;
	wstr.resize(str.length());
	mbstowcs_s(&size, &wstr[0], wstr.size() + 1, str.c_str(), str.size());
	return wstr;
}

string IDPipeMessanger::WStringToString(const std::wstring& wstr)
{
	string str;
	size_t size;
	str.resize(wstr.length());
	wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
	return str;
}


string IDPipeMessanger::GetString(PacketHeader header, std::shared_ptr<HeapAllocatedData> heapData) {

	if (header.DataSize() == 0)
		return string();

	if (GetStringType(header) == DP_STRING_TYPE::WSTRING) {
		wstring result((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
		return WStringToString(result);
	}

	return string((char*)heapData->data(), heapData->size());
}

string IDPipeMessanger::GetString(PacketHeader header) {

	auto dataSize = header.DataSize();

	if (dataSize == 0)
		return string();

	HeapAllocatedData data(dataSize, false);
	_dpipe->Read(data.data(), dataSize);

	if (GetStringType(header) == DP_STRING_TYPE::WSTRING) {
		wstring result((wchar_t*)data.data(), dataSize / sizeof(wchar_t));
		return WStringToString(result);
	}

	return string((char*)data.data(), dataSize);
}

string IDPipeMessanger::GetString(std::shared_ptr<HeapAllocatedData> heapData, int encodingCode) {
	if (heapData->size() == 0)
		return string();

	if (GetStringType(encodingCode) == DP_STRING_TYPE::WSTRING) {
		wstring result((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
		return WStringToString(result);
	}

	return string((char*)heapData->data(), heapData->size());
}

wstring IDPipeMessanger::GetWString(PacketHeader header, std::shared_ptr<HeapAllocatedData> heapData) {
	if (header.DataSize() == 0)
		return wstring();

	if (GetStringType(header) == DP_STRING_TYPE::STRING) {
		string result ((char*)heapData->data(), heapData->size());
		return StringToWString(result);
	}

	return wstring((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
}

wstring IDPipeMessanger::GetWString(PacketHeader header) {

	auto dataSize = header.DataSize();

	if (dataSize == 0)
		return wstring();

	HeapAllocatedData data(dataSize, false);
	_dpipe->Read(data.data(), dataSize);

	if (GetStringType(header) == DP_STRING_TYPE::STRING) {
		string result((char*)data.data(), dataSize);
		return StringToWString(result);
	}

	return wstring((wchar_t*)data.data(), dataSize / sizeof(wchar_t));
}

wstring IDPipeMessanger::GetWString(std::shared_ptr<HeapAllocatedData> heapData, int encodingCode)
{
	if (heapData->size() == 0)
		return wstring();

	if (GetStringType(encodingCode) == DP_STRING_TYPE::STRING) {
		string result((char*)heapData->data(), heapData->size());
		return StringToWString(result);
	}

	return wstring((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
}

bool IDPipeMessanger::Send(DP_MESSAGE_TYPE messageType, void* data, DWORD dataSize) {

	DWORD dataCode = 0;

	switch (messageType) {
		case DP_MESSAGE_TYPE::MESSAGE:
			dataCode = DP_MESSAGE_DATA;
			break;
		case DP_MESSAGE_TYPE::MESSAGE_INFO:
			dataCode = DP_INFO_DATA;
			break;
		case DP_MESSAGE_TYPE::MESSAGE_WARNING:
			dataCode = DP_WARNING_DATA;
			break;
		case DP_MESSAGE_TYPE::MESSAGE_ERROR:
			dataCode = DP_ERROR_DATA;
			break;
		default:
			throw exception("Unkonws message type");
	}

	DWORD nBytesWritten;

	_mutexWrite.lock();
		bool result = _dpipe->Write(dataCode, data, dataSize, &nBytesWritten);
	_mutexWrite.unlock();

	return result;
}

bool IDPipeMessanger::SendDataStatic(LPVOID dataPtr) {
	SendDataAsyncContainer* sendData = (SendDataAsyncContainer*)dataPtr;
	bool result = sendData->dpMessanger->Send(sendData->type, sendData->data, sendData->size);

	if (sendData->callback) {
		sendData->callback(sendData->dpMessanger->_dpipe);
	}

	return result;
}

bool IDPipeMessanger::SendAsync(DP_MESSAGE_TYPE messageType, void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback) {
	SendDataAsyncContainer* sendData = new SendDataAsyncContainer(this, messageType, data, dataSize, callback);
	DWORD _nReadThreadId;
	auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataStatic, (LPVOID)sendData, 0, &_nReadThreadId);
	return _tHandleThread != INVALID_HANDLE_VALUE;
}

bool crdk::dpipes::IDPipeMessanger::Send(std::string message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE, message);
}

bool IDPipeMessanger::Send(DP_MESSAGE_TYPE messageType, std::string message)
{
	DWORD dataCode = 0;

	switch (messageType) {
	case DP_MESSAGE_TYPE::MESSAGE:
		dataCode = AddEncodingCode(DP_MESSAGE_STRING, _stringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_INFO:
		dataCode = AddEncodingCode(DP_INFO_STRING, _stringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_WARNING:
		dataCode = AddEncodingCode(DP_WARNING_STRING, _stringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_ERROR:
		dataCode = AddEncodingCode(DP_ERROR_STRING, _stringEncodingCode);
		break;
	default:
		throw exception("Unkonws message type");
	}

	DWORD nBytesWritten;
	DWORD len = boost::numeric_cast<DWORD>(message.length());

	_mutexWrite.lock();
		bool result = _dpipe->Write(dataCode, message.data(), len, &nBytesWritten);
	_mutexWrite.unlock();

	return result;
}

bool IDPipeMessanger::SendStringStatic(LPVOID dataPtr) {
	SendStringAsyncContainer* sendData = (SendStringAsyncContainer*)dataPtr;
	bool result = sendData->dpMessanger->Send(sendData->type, sendData->data);

	if (sendData->callback) {
		sendData->callback(sendData->dpMessanger->_dpipe);
	}

	return result;
}

bool IDPipeMessanger::SendAsync(DP_MESSAGE_TYPE messageType, std::string message, std::function<void(IDPipe* pipe)> callback) {
	SendStringAsyncContainer* sendData = new SendStringAsyncContainer(this, messageType, message, callback);
	DWORD _nReadThreadId;
	auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendStringStatic, (LPVOID)sendData, 0, &_nReadThreadId);
	return _tHandleThread != INVALID_HANDLE_VALUE;
}

IDPipe* crdk::dpipes::IDPipeMessanger::GetPipe() const{
	return _dpipe;
}

bool crdk::dpipes::IDPipeMessanger::Send(std::wstring message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE, message);
}

bool IDPipeMessanger::Send(DP_MESSAGE_TYPE messageType, std::wstring message)
{
	DWORD dataCode = 0;

	switch (messageType) {
	case DP_MESSAGE_TYPE::MESSAGE:
		dataCode = AddEncodingCode(DP_MESSAGE_STRING, _wstringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_INFO:
		dataCode = AddEncodingCode(DP_INFO_STRING, _wstringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_WARNING:
		dataCode = AddEncodingCode(DP_WARNING_STRING, _wstringEncodingCode);
		break;
	case DP_MESSAGE_TYPE::MESSAGE_ERROR:
		dataCode = AddEncodingCode(DP_ERROR_STRING, _wstringEncodingCode);
		break;
	default:
		throw exception("Unkonws message type");
	}

	DWORD nBytesWritten;
	DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));

	_mutexWrite.lock();
		bool result = _dpipe->Write(dataCode, message.data(), len, &nBytesWritten);
	_mutexWrite.unlock();

	return result;

}

bool IDPipeMessanger::SendWStringStatic(LPVOID dataPtr) {
	SendWStringAsyncContainer* sendData = (SendWStringAsyncContainer*)dataPtr;
	bool result = sendData->dpMessanger->Send(sendData->type, sendData->data);

	if (sendData->callback) {
		sendData->callback(sendData->dpMessanger->_dpipe);
	}

	return result;
}

bool IDPipeMessanger::SendAsync(DP_MESSAGE_TYPE messageType, std::wstring message, std::function<void(IDPipe* pipe)> callback) {
	SendWStringAsyncContainer* sendData = new SendWStringAsyncContainer(this, messageType, message, callback);
	DWORD _nReadThreadId;
	auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendWStringStatic, (LPVOID)sendData, 0, &_nReadThreadId);
	return _tHandleThread != INVALID_HANDLE_VALUE;
}

bool IDPipeMessanger::SendMessageString(const std::string message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE, message);
}

bool IDPipeMessanger::SendMessageString(const std::wstring message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE, message);
}

bool IDPipeMessanger::SendMessageStringAsync(const std::string message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE, message, callback);
}

bool IDPipeMessanger::SendMessageStringAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE, message, callback);
}

bool IDPipeMessanger::SendMessageData(void* data, DWORD dataSize) {
	return Send(DP_MESSAGE_TYPE::MESSAGE, data, dataSize);
}

bool IDPipeMessanger::SendMessageDataAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE, data, dataSize, callback);
}

bool IDPipeMessanger::SendInfo(const std::string message){
	return Send(DP_MESSAGE_TYPE::MESSAGE_INFO, message);
}

bool IDPipeMessanger::SendInfo(const std::wstring message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_INFO, message);
}

bool IDPipeMessanger::SendInfoAsync(const std::string message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_INFO, message);
}

bool IDPipeMessanger::SendInfoAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_INFO, message);
}

bool IDPipeMessanger::SendInfo(void* data, DWORD dataSize) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_INFO, data, dataSize);
}

bool IDPipeMessanger::SendInfoAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_INFO, data, dataSize, callback);;
}

bool IDPipeMessanger::SendWarning(const std::string message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_WARNING, message);
}

bool IDPipeMessanger::SendWarning(const std::wstring message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_WARNING, message);
}

bool IDPipeMessanger::SendWarningAsync(const std::string message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_WARNING, message, callback);
}

bool IDPipeMessanger::SendWarningAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_WARNING, message, callback);
}

bool IDPipeMessanger::SendWarning(void* data, DWORD dataSize) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_WARNING, data, dataSize);
}

bool IDPipeMessanger::SendWarningAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback){
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_WARNING, data, dataSize, callback);
}

bool IDPipeMessanger::SendError(const std::string message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_ERROR, message);
}

bool IDPipeMessanger::SendError(const std::wstring message) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_ERROR, message);
}

bool IDPipeMessanger::SendErrorAsync(const std::string message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_ERROR, message, callback);
}

bool IDPipeMessanger::SendErrorAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_ERROR, message, callback);
}

bool IDPipeMessanger::SendError(void* data, DWORD dataSize) {
	return Send(DP_MESSAGE_TYPE::MESSAGE_ERROR, data, dataSize);
}

bool IDPipeMessanger::SendErrorAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback) {
	return SendAsync(DP_MESSAGE_TYPE::MESSAGE_ERROR, data, dataSize, callback);
}

void IDPipeMessanger::SetMessageStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onMessageStringReceived = function;
}

void IDPipeMessanger::SetMessageDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onMessageDataReceived = function;
}

void IDPipeMessanger::SetInfoStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onInfoStringReceived = function;
}

void IDPipeMessanger::SetInfoDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onInfoDataReceived = function;
}

void IDPipeMessanger::SetWarningStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onWarningStringReceived = function;
}

void IDPipeMessanger::SetWarningDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onWarningDataReceived = function;
}

void IDPipeMessanger::SetErrorStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onErrorStringReceived = function;
}

void IDPipeMessanger::SetErrorDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData>data)> function) {
	_onErrorDataReceived = function;
}

void IDPipeMessanger::SetStringEncodingCode(DWORD code) {
	_stringEncodingCode = code;
}

void IDPipeMessanger::SetWStringEncodingCode(DWORD code) {
	_wstringEncodingCode = code;
}

void IDPipeMessanger::Disconnect() {
	_dpipe->Disconnect();
}

void IDPipeMessanger::Disconnect(std::string message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length());
	_dpipe->Disconnect(message.data(), len, DP_ENCODING_UTF8);
}

void IDPipeMessanger::Disconnect(std::wstring message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length() * (sizeof(wchar_t)));
	_dpipe->Disconnect(message.data(), len, DP_ENCODING_UNICODE);
}
