#include "dpmessanger.h"

using namespace std;
using namespace crdk::dpipes;

//DPMessageReceivedAsyncData implementation
#pragma region DPMessageReceivedAsyncData
	DPMessageReceivedAsyncData::DPMessageReceivedAsyncData(IDPipeMessanger* messangerPtr, PacketHeader packetHeader, std::shared_ptr<HeapAllocatedData> heapData) :
		header(packetHeader) {
		messanger = messangerPtr;
		data = heapData;
	}
#pragma endregion DPMessageReceivedAsyncData

//DPMessageStringReceivedAsyncData implementation
#pragma region DPMessageStringReceivedAsyncData
	DPMessageStringReceivedAsyncData::DPMessageStringReceivedAsyncData(IDPipeMessanger* messangerPtr, PacketHeader packetHeader, std::wstring messageString) :
		header(packetHeader) {
		messanger = messangerPtr;
		message = messageString;
	}
#pragma endregion DPMessageStringReceivedAsyncData

//IDPipeMessanger implementation
#pragma region IDPipeMessanger
	IDPipeMessanger::IDPipeMessanger(IDPipe* dpipe, DWORD nBufferStringSize) {
		_dpipe = dpipe;

		if (_dpipe == nullptr)
			return;

		_nBufferStringSize = nBufferStringSize;
		_pBufferString = (wchar_t*)(new char[_nBufferStringSize]);
	}

	IDPipeMessanger::~IDPipeMessanger() {
		delete[] _pBufferString;
	}

	void IDPipeMessanger::ChekBufferSize(DWORD nBytes) {
		if (nBytes > _nBufferStringSize) {
			delete[] _pBufferString;
			_nBufferStringSize = nBytes + (nBytes / 2);
			_pBufferString = (wchar_t*)(new char[_nBufferStringSize]);
		}
	}

	wstring IDPipeMessanger::GetStringFromPipe(DWORD sizeInBytes) {
		if (sizeInBytes > 0) {
			ChekBufferSize(sizeInBytes);

			DWORD nBytesRead;
			_dpipe->Read(_pBufferString, sizeInBytes, &nBytesRead, NULL);
			_pBufferString[sizeInBytes / sizeof(wchar_t)] = '\0';

			return wstring(_pBufferString);
		}
		else {
			return L"";
		}
	}

	void IDPipeMessanger::OnMessageReceived(PacketHeader& header, shared_ptr<HeapAllocatedData> heapData) {

		switch (header.GetCommand())
		{
		case DP_MESSAGE_DATA:
			if (_onMessageDataReceived)
				_onMessageDataReceived(heapData);
			break;
		case DP_INFO_DATA:
			if (_onInfoDataReceived)
				_onInfoDataReceived(heapData);
			break;
		case DP_WARNING_DATA:
			if (_onWarningDataReceived)
				_onWarningDataReceived(heapData);
			break;
		case DP_ERROR_DATA:
			if (_onErrorDataReceived)
				_onErrorDataReceived(heapData);
			break;
		}
	}

	void IDPipeMessanger::OnMessageReceivedAsync(LPVOID dataPtr) {
		auto messageData = (DPMessageReceivedAsyncData*)dataPtr;
		IDPipeMessanger* messanger = messageData->messanger;
		messanger->OnMessageReceived(messageData->header, messageData->data);
		delete messageData;
	}

	void IDPipeMessanger::OnMessageStringReceived(PacketHeader& header, wstring stringMessage) {
		switch (header.GetCommand())
		{
		case DP_MESSAGE_STRING:
			if (_onMessageStringReceived)
				_onMessageStringReceived(stringMessage);
			break;
		case DP_INFO_STRING:
			if (_onInfoStringReceived)
				_onInfoStringReceived(stringMessage);
			break;
		case DP_WARNING_STRING:
			if (_onWarningStringReceived)
				_onWarningStringReceived(stringMessage);
			break;
		case DP_ERROR_STRING:
			if (_onErrorStringReceived)
				_onErrorStringReceived(stringMessage);
			break;
		}
	}

	void IDPipeMessanger::OnMessageStringReceivedAsync(LPVOID dataPtr) {
		auto messageData = (DPMessageStringReceivedAsyncData*)dataPtr;
		IDPipeMessanger* messanger = messageData->messanger;
		messanger->OnMessageStringReceived(messageData->header, messageData->message);
		delete messageData;
	}

	bool IDPipeMessanger::SendMSG(const wstring message) {
		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_MESSAGE_STRING, message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendMSG(void* data, DWORD dataSize) {
		DWORD nBytesWritten;
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_MESSAGE_DATA, data, dataSize, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendInfo(const wstring message) {
		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_INFO_STRING, message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendInfo(void* data, DWORD dataSize) {
		DWORD nBytesWritten;
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_INFO_DATA, data, dataSize, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendWarning(const wstring message) {
		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_WARNING_STRING, message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendWarning(void* data, DWORD dataSize) {
		DWORD nBytesWritten;
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_WARNING_DATA, data, dataSize, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendError(const wstring message) {
		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_ERROR_STRING, message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	bool IDPipeMessanger::SendError(void* data, DWORD dataSize) {
		DWORD nBytesWritten;
		_mutexWritePipe.lock();
			bool bReturn = _dpipe->Write(DP_ERROR_DATA, data, dataSize, &nBytesWritten);
		_mutexWritePipe.unlock();
		return bReturn;
	}

	void IDPipeMessanger::SetMessageStringReceivedCallback(function<void(wstring)> function) {
		_onMessageStringReceived = function;
	}

	void IDPipeMessanger::SetMessageDataReceivedCallback(function<void(shared_ptr<HeapAllocatedData> data)> function) {
		_onMessageDataReceived = function;
	}

	void IDPipeMessanger::SetInfoStringReceivedCallback(function<void(wstring)> function) {
		_onInfoStringReceived = function;
	}

	void IDPipeMessanger::SetInfoDataReceivedCallback(function<void(shared_ptr<HeapAllocatedData> data)> function) {
		_onInfoDataReceived = function;
	}

	void IDPipeMessanger::SetWarningStringReceivedCallback(function<void(wstring)> function) {
		_onWarningStringReceived = function;
	}

	void IDPipeMessanger::SetWarningDataReceivedCallback(function<void(shared_ptr<HeapAllocatedData> data)> function) {
		_onWarningDataReceived = function;
	}

	void IDPipeMessanger::SetErrorStringReceivedCallback(function<void(wstring)> function) {
		_onErrorStringReceived = function;
	}

	void IDPipeMessanger::SetErrorDataReceivedCallback(function<void(shared_ptr<HeapAllocatedData> data)> function) {
		_onErrorDataReceived = function;
	}

#pragma endregion IDPipeMessanger