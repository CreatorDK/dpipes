#include "dpwstring.h"

using namespace std;
using namespace crdk::dpipes;

//Static method that execute in different thread for async operations
#pragma region RunHandleMessageWStringAsync
	void crdk::dpipes::RunHandleMessageWStringAsync(LPVOID dataPtr) {
		WStringCallbackDataAsync* callBackData = (WStringCallbackDataAsync*)dataPtr;

		if (callBackData->callback)
			callBackData->callback(callBackData->data);

		delete callBackData;
	}
#pragma endregion RunHandleMessageWStringAsync

//DPWString implementation
#pragma region DPWString
	DPWString::DPWString(IDPipe* dpipe, bool handleAsync, DWORD nBufferSize) {

		_dpipe = dpipe;

		if (dpipe == nullptr)
			return;

		_handleAsync = handleAsync;
		_nBufferSize = nBufferSize;
		_buffer = (wchar_t*)(new char[_nBufferSize]);

		dpipe->SetClientConnectCallback([=](PacketHeader header) { this->OnClientConnect(header); });
		dpipe->SetOtherSideDisconnectCallback([=](PacketHeader header) { this->OnOtherSideDisconnect(header); });
		dpipe->SetPacketHeaderRecevicedCallback([=](PacketHeader header) { this->OnMessageReceived(header); });
	}

	DPWString::~DPWString()
	{
		_dpipe->SetClientConnectCallback(nullptr);
		_dpipe->SetOtherSideDisconnectCallback(nullptr);
		_dpipe->SetPacketHeaderRecevicedCallback(nullptr);

		_nBufferSize = 0;
		delete[] _buffer;
	}

	wstring DPWString::GetStringFromPipe(const PacketHeader& header) {
		if (header.DataSize() > 0) {
			ChekBufferSize(header.DataSize());

			DWORD nBytesRead;
			_dpipe->Read(_buffer, header.DataSize(), &nBytesRead, NULL);
			DWORD charactersNumber = header.DataSize() / sizeof(wchar_t);
			_buffer[charactersNumber] = '\0';

			wstring result = wstring(_buffer, charactersNumber);

			return result;
		}
		else {
			return L"";
		}
	}

	void DPWString::SendAsync(LPVOID dataPtr) {
		SendWStringDataAsync* sendData = (SendWStringDataAsync*)dataPtr;

		auto dpstring = sendData->dpstring;
		dpstring->Send(sendData->data);

		if (sendData->callback)
			sendData->callback();
	}

	void DPWString::StaticSendWStringAsync(LPVOID dataPtr) {
		SendWStringDataAsync* sendData = (SendWStringDataAsync*)dataPtr;

		auto dpwstring = sendData->dpstring;
		dpwstring->Send(sendData->data);

		if (sendData->callback)
			sendData->callback();

		delete sendData;
	}

	void DPWString::OnClientConnect(PacketHeader header) {
		wstring message = GetStringFromPipe(header);

		if (_onClientConnected)
			_onClientConnected(message);
	}

	void DPWString::OnOtherSideDisconnect(PacketHeader header) {
		wstring message = GetStringFromPipe(header);

		if (_onOtherSideDisconnected)
			_onOtherSideDisconnected(message);
	}

	void DPWString::OnMessageReceived(PacketHeader header) {
		wstring message = GetStringFromPipe(header);

		if (_onMessageReceived) {
			if (_handleAsync) {
				WStringCallbackDataAsync* callackData = new WStringCallbackDataAsync;
				callackData->callback = _onMessageReceived;
				callackData->data = message;

				DWORD _nReadThreadId;
				auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RunHandleMessageWStringAsync, (LPVOID)callackData, 0, &_nReadThreadId);
			}
			else 
				_onMessageReceived(message);
		}	
	}

	void DPWString::ChekBufferSize(DWORD nBytes) {
		if (nBytes > _nBufferSize) {
			delete[] _buffer;
			_nBufferSize = nBytes + (nBytes / 2);
			_buffer = (wchar_t*)(new char[_nBufferSize]);
		}
	}

	void DPWString::SetOnClientConnectHandler(function<void(wstring)> function) {
		_onClientConnected = function;
	}

	void DPWString::SetOnOtherSideDisconnectHandler(function<void(wstring)> function) {
		_onOtherSideDisconnected = function;
	}

	void DPWString::SetOnMessageReceivedHandler(function<void(wstring)> function) {
		_onMessageReceived = function;
	}

	void DPWString::Send(wstring message)
	{
		if (_dpipe == nullptr)
			return;

		DWORD nBytesWritten;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));

		_mutexWritePipe.lock();
			_dpipe->Write(message.data(), len, &nBytesWritten);
		_mutexWritePipe.unlock();
	}

	void DPWString::SendAsync(wstring message, function<void(void)> callback) {
		SendWStringDataAsync* sendData = new SendWStringDataAsync();
		sendData->dpstring = this;
		sendData->data = message;
		sendData->callback = callback;

		DWORD _nReadThreadId;
		auto _tHandleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StaticSendWStringAsync, (LPVOID)sendData, 0, &_nReadThreadId);
	}

	bool DPWString::Connect(IDPipeHandle* pHandle, std::wstring connectString) {
		if (connectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(connectString.length() * sizeof(wchar_t));
			return _dpipe->Connect(pHandle, connectString.data(), len);
		}
		else
			return _dpipe->Connect(pHandle);
	}

	bool DPWString::Connect(std::wstring handle, std::wstring connectString) {
		if (connectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(connectString.length() * sizeof(wchar_t));
			return _dpipe->Connect(handle, connectString.data(), len);
		}
		else
			return _dpipe->Connect(handle);
	}

	void DPWString::Disconnect(std::wstring disconnectString) {
		if (disconnectString.length() > 0) {
			DWORD len = boost::numeric_cast<DWORD>(disconnectString.length() * sizeof(wchar_t));
			_dpipe->Disconnect(disconnectString.data(), len);
		}
		else
			_dpipe->Disconnect();
	}
#pragma endregion DPWString