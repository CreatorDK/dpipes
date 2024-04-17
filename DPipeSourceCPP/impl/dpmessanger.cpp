#include "dpmessanger.h"

using namespace std;
using namespace crdk::dpipes;

DPMessanger::DPMessanger(IDPipe* dpipe, bool handleAsync) :
	IDPipeMessanger(dpipe)
{
	_handleAsync = handleAsync;
	_dpipe->SetClientConnectCallback([this](IDPipe* pipe, PacketHeader header) { OnClientConnect(pipe, header); });
	_dpipe->SetOtherSideDisconnectCallback([this](IDPipe* pipe, PacketHeader header) { OnOtherSideDisconnect(pipe, header); });
	_dpipe->SetPacketHeaderRecevicedCallback([this](IDPipe* pipe, PacketHeader header) { OnDataReceive(pipe, header); });
}

DPMessanger::~DPMessanger() {
	_dpipe->SetClientConnectCallback({});
	_dpipe->SetOtherSideDisconnectCallback({});
	_dpipe->SetPacketHeaderRecevicedCallback({});
}

void DPMessanger::OnClientConnect(IDPipe* pipe, PacketHeader header) {
	
	if (_onClientConnected) {
		DWORD dataSize = header.DataSize();
		auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

		if (dataSize > 0) {
			void* buffer = heapData.get()->data();
			DWORD nBytesRead;
			_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
		}

		_onClientConnected(pipe, header, heapData);
	}
}

void DPMessanger::OnOtherSideDisconnect(IDPipe* pipe, PacketHeader header) {
	
	if (_onOtherSideDisconnected) {
		DWORD dataSize = header.DataSize();
		auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

		if (dataSize > 0) {
			void* buffer = heapData.get()->data();
			DWORD nBytesRead;
			_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
		}

		_onOtherSideDisconnected(pipe, header, heapData);
	}
}

void DPMessanger::OnDataReceive(IDPipe* pipe, PacketHeader header) {

	DWORD nBytesRead;
	//Get data code ignoring first 8 bits (using for encoding)
	DWORD dataCode = header.GetDataCodeOnly();
	bool isStringData = dataCode & 0x01;

	DWORD dataSize = header.DataSize();
	auto heapData = make_shared<HeapAllocatedData>(dataSize, false);

	if (dataSize > 0) {
		void* buffer = heapData.get()->data();
		_dpipe->Read(buffer, header.DataSize(), &nBytesRead, NULL);
	}

	//If data is string
	if (isStringData) {

		if (_handleAsync) {
			DWORD _nReadThreadId;
			DPMessageReceivedAsyncData* sendData = new DPMessageReceivedAsyncData(this, header, heapData, _dpipe);
			auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageStringReceivedAsync, (LPVOID)sendData, 0, &_nReadThreadId);
		}
		else {
			OnMessageStringReceived(_dpipe, header, heapData);
		}
	}
	//If data is binary
	else if (dataCode > 0) {
		if (_handleAsync) {
			DWORD _nReadThreadId;
			DPMessageReceivedAsyncData* sendData = new DPMessageReceivedAsyncData(this, header, heapData, _dpipe);
			auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMessageStringReceivedAsync, (LPVOID)sendData, 0, &_nReadThreadId);
		}
		else {
			OnMessageReceived(_dpipe, header, heapData);
		}
	}
	else {
		if (_onMessageStringReceived)
			_onMessageStringReceived(pipe, header, heapData);
	}
}

bool DPMessanger::Connect(IDPipeHandle* pHandle) {
	return _dpipe->Connect(pHandle);
}

bool DPMessanger::Connect(IDPipeHandle* pHandle, std::string message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length());
	return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UTF8);
}

bool DPMessanger::Connect(IDPipeHandle* pHandle, std::wstring message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
	return _dpipe->Connect(pHandle, message.data(), len, DP_ENCODING_UNICODE);
}

bool DPMessanger::Connect(const std::wstring handleString) {
	return _dpipe->Connect(handleString);
}

bool DPMessanger::Connect(const std::wstring handleString, std::string message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length());
	return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UTF8);
}

bool DPMessanger::Connect(const std::wstring handleString, std::wstring message) {
	DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
	return _dpipe->Connect(handleString, message.data(), len, DP_ENCODING_UNICODE);
}

void DPMessanger::SetOnClientConnectHandler(function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function) {
	_onClientConnected = function;
}

void DPMessanger::SetOnOtherSideDisconnectHandler(function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function) {
	_onOtherSideDisconnected = function;
}