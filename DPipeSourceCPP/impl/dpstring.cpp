#include "dpstring.h"

using namespace std;
using namespace crdk::dpipes;

//Static method that execute in different thread for async operations
#pragma region RunHandleMessageStringAsync
	static void crdk::dpipes::RunHandleMessageStringAsync(LPVOID data) {
		StringCallbackDataAsync* callBackData = (StringCallbackDataAsync*)data;

		if (callBackData->callback)
			callBackData->callback(callBackData->data);

		delete callBackData;
	}
#pragma endregion RunHandleMessageStringAsync

//DPString implementation
#pragma region DPString
	DPString::DPString(IDPipe* dpipe, bool handleAsync, DWORD nBufferSize) {

		_dpipe = dpipe;

		if (dpipe == nullptr)
			return;

		_handleAsync = handleAsync;
		_nBufferSize = nBufferSize;
		_buffer = new char [_nBufferSize];

		dpipe->SetClientConnectCallback([=](PacketHeader header) { this->OnClientConnect(header); });
		dpipe->SetOtherSideDisconnectCallback([=](PacketHeader header) { this->OnOtherSideDisconnect(header); });
		dpipe->SetPacketHeaderRecevicedCallback([=](PacketHeader header) { this->OnMessageReceived(header); });
	}

	DPString::~DPString()
	{
		_dpipe->SetClientConnectCallback(nullptr);
		_dpipe->SetOtherSideDisconnectCallback(nullptr);
		_dpipe->SetPacketHeaderRecevicedCallback(nullptr);

		_nBufferSize = 0;
		delete[] _buffer;
	}

	string DPString::GetStringFromPipe(const PacketHeader& header) {
		DWORD dataSize = header.DataSize();

		if (dataSize > 0) {
			ChekBufferSize(dataSize);

			DWORD nBytesRead;
			_dpipe->Read(_buffer, dataSize, &nBytesRead, NULL);
			return string(_buffer, header.DataSize());
		}
		else {
			return "";
		}
	}

	void DPString::StaticSendStringAsync(LPVOID dataPtr) {
		SendStringDataAsync* sendData = (SendStringDataAsync*)dataPtr;

		auto dpstring = sendData->dpstring;
		dpstring->Send(sendData->data);

		if (sendData->callback)
			sendData->callback();

		delete sendData;
	}

	void DPString::OnClientConnect(PacketHeader header) {
		string message = GetStringFromPipe(header);

		if (_onClientConnected)
			_onClientConnected(message);
	}

	void DPString::OnOtherSideDisconnect(PacketHeader header) {
		string message = GetStringFromPipe(header);

		if (_onOtherSideDisconnected)
			_onOtherSideDisconnected(message);
	}

	void DPString::OnMessageReceived(PacketHeader header) {

		string message = GetStringFromPipe(header);

		if (_onMessageReceived) {

			if (_handleAsync) {
				StringCallbackDataAsync* callackData = new StringCallbackDataAsync;
				callackData->callback = _onMessageReceived;
				callackData->data = message;

				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunHandleMessageStringAsync, (LPVOID)callackData, 0, &_nReadThreadId);
			}
			else 
				_onMessageReceived(message);
		}
	}

	void DPString::ChekBufferSize(DWORD nBytes) {
		if (nBytes > _nBufferSize) {
			delete[] _buffer;
			_nBufferSize = nBytes + (nBytes / 2);
			_buffer = (char*)(new char[_nBufferSize]);
		}
	}

	void DPString::SetOnClientConnectHandler(function<void(string)> function) {
		_onClientConnected = function;
	}

	void DPString::SetOnOtherSideDisconnectHandler(function<void(string)> function) {
		_onOtherSideDisconnected = function;
	}

	void DPString::SetOnMessageReceivedHandler(function<void(string)> function) {
		_onMessageReceived = function;
	}

	void DPString::Send(string message)
	{
		if (_dpipe == nullptr)
			return;

		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length());

		_mutexWritePipe.lock();
			_dpipe->Write(message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
	}
	void DPString::SendAsync(string message, function<void(void)> callback) {
		SendStringDataAsync* sendData = new SendStringDataAsync();
		sendData->dpstring = this;
		sendData->data = message;
		sendData->callback = callback;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StaticSendStringAsync, (LPVOID)sendData, 0, &_nReadThreadId);
	}
	bool DPString::Connect(IDPipeHandle* pHandle, std::string connectString) {
		if (connectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(connectString.length());
			return _dpipe->Connect(pHandle, connectString.data(), len);
		}
		else
			return _dpipe->Connect(pHandle);
	}

	bool DPString::Connect(wstring handle, std::string connectString) {
		if (connectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(connectString.length());
			return _dpipe->Connect(handle, connectString.data(), len);
		}
		else
			return _dpipe->Connect(handle);
	}

	void DPString::Disconnect(std::string disconnectString) {
		if (disconnectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(disconnectString.length());
			_dpipe->Disconnect(disconnectString.data(), len);
		}
		else
			_dpipe->Disconnect();
	}

#pragma endregion DPString