#pragma once
#include "dpipebase.h"
#include <stdexcept>
#include <iostream>
#include <vector>

using namespace std;
using namespace crdk::dpipes;

//IDuplexPipeHandle class implementation
#pragma region IDPipeHandle
	IDPipeHandle::IDPipeHandle() { }
#pragma endregion IDPipeHandle

//IDuplexPipeHandle class implementation
#pragma region IDPipeHandle
	IDPipeHandle::~IDPipeHandle() {}
#pragma endregion IDPipeHandle

//IDuplexPipe base class implementation
#pragma region IDPipe
	IDPipe::IDPipe(const std::wstring& sName, 
		DWORD mtu,
		DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		_mtu(mtu),
		_sName(sName),
		_nInBufferSize(nInBufferSize),
		_nOutBufferSize(nOutBufferSize) { 
		_skipBuffer = new char[_skipBufferSize];
	}

	IDPipe::~IDPipe() { 
		if (_mode != DP_MODE::UNSTARTED)
			Disconnect();
	}

	std::wstring IDPipe::GetName() const {
		return _sName; 
	}

	DWORD IDPipe::BytesToRead() const {
		return _nBytesToRead;
	}

	DWORD IDPipe::GetLastErrorDP() const {
		return _nLastError;
	}

	void IDPipe::Disconnect(LPCVOID pDisconnectData, DWORD nDisconnectDataSize, DWORD prefix) {

		if (_mode == DP_MODE::UNSTARTED)
			return;

		if (_mode == DP_MODE::INNITIATOR && !_bIsAlive) {
			_onPingReceivedCallBack = nullptr;
			_onPongReceivedCallBack = nullptr;
			_onOtherSideConnectCallBack = nullptr;
			_onOtherSideDisconnectCallBack = nullptr;
			_onPacketHeaderReceivedCallBack = nullptr;

			_clientEmulating = true;

			auto handle = GetHandle();
			auto virtualClient = CreateNewInstance();
			virtualClient->_clientEmulating = true;
			virtualClient->Connect(handle.get());
			virtualClient->Disconnect();
			while (virtualClient->Mode() != DP_MODE::UNSTARTED) {
				Sleep(1);
			}
			delete virtualClient;
			return;
		}

		if ((!_bOtherSideDisconnecting && _bIsAlive)) {
			PacketHeader header(true, nDisconnectDataSize);
			header.SetServiceCode(DP_SERVICE_CODE_DISCONNECT);
			header.SetServicePrefix(prefix);

			_packetPuilder.PrepareHeader(header);
			_packetPuilder.WriteHeader(_hWritePipe);

			if (nDisconnectDataSize) {
				DWORD nBytesWritten;
				WriteRaw(pDisconnectData, nDisconnectDataSize, &nBytesWritten);
			}
		}

		if (_tReadThread != nullptr && _bIsAlive) {
			WaitForSingleObject(_tReadThread, INFINITE);
			CloseHandle(_tReadThread);
			_tReadThread = nullptr;
		}
		else {
			DisconnectPipe(_bIsAlive);
		}

		_mode = DP_MODE::UNSTARTED;
	}

	void IDPipe::Disconnect() {
		Disconnect(nullptr, 0);
	}

	void crdk::dpipes::IDPipe::SetPingReceivedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function) {
		_onPingReceivedCallBack = function;
	}

	void crdk::dpipes::IDPipe::SetPongReceivedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function) {
		_onPongReceivedCallBack = function;
	}

	void IDPipe::SetClientConnectCallback(std::function<void(IDPipe* sender, PacketHeader header)> function) {
		_onOtherSideConnectCallBack = function;
	}

	void IDPipe::SetOtherSideDisconnectCallback(std::function<void(IDPipe* sender, PacketHeader header)> function) {
		_onOtherSideDisconnectCallBack = function;
	}

	void IDPipe::SetPacketHeaderRecevicedCallback(std::function<void(IDPipe* sender, PacketHeader header)> function){
		_onPacketHeaderReceivedCallBack = function;
	}

	void crdk::dpipes::IDPipe::SetConfigurationRecevicedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function) {
		_onConfigurationReceivedCallBack = function;
	}

	void IDPipe::Skip(DWORD nBytesToSkip) {

		if (nBytesToSkip == 0)
			return;

		if (nBytesToSkip > _nBytesToRead)
			throw exception("Unable to skip more bytes then specified in PacketHeader");

		int cycles = nBytesToSkip / _skipBufferSize;
		DWORD restBytes = nBytesToSkip % _skipBufferSize;

		DWORD nBytesRead;

		for (int i = 0; i < cycles; i++) {
			Read(_skipBuffer, _skipBufferSize, &nBytesRead, NULL);
		}

		if (restBytes > 0) {
			Read(_skipBuffer, restBytes, &nBytesRead, NULL);
		}
	}

	void IDPipe::Skip(PacketHeader header) {
		Skip(header.DataSize());
	}

	HANDLE IDPipe::ReadHandle() {
		return _hReadPipe;
	}

	HANDLE IDPipe::WriteHandle() {
		return _hWritePipe;
	}

	void IDPipe::OnClientConnect(PacketHeader ph) {
		OnPipeClientConnect();

		_bIsAlive = true;

		if (_onOtherSideConnectCallBack) {
			_onOtherSideConnectCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}
		else {
			if (ph.DataSize() > 0)
				Skip(ph);
		}
	}

	void IDPipe::OnOtherSideDisconnect(PacketHeader ph) {

		if (_onOtherSideDisconnectCallBack) {
			_onOtherSideDisconnectCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}
		else {
			if (ph.DataSize() > 0)
				Skip(ph);
		}

		_bOtherSideDisconnecting = true;
	}

	void IDPipe::OnOtherSideDisconnectPipe() {
		_bIsAlive = false;
		_bListening = false;

		DisconnectPipe(_bIsAlive);
		_mode = DP_MODE::UNSTARTED;
	}

	void IDPipe::OnTerminating(PacketHeader ph) { }

	void IDPipe::OnPacketHeaderReceived(PacketHeader ph) {
		if (_onPacketHeaderReceivedCallBack) {
			_onPacketHeaderReceivedCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}

		else {
			Skip(ph);
		}
	}

	void IDPipe::OnPingReceived(PacketHeader ph) {

		SendPong(_hWritePipe);

		if (_onPingReceivedCallBack) {
			_onPingReceivedCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}

		else
			Skip(ph);
	}

	void IDPipe::OnPongReceived(PacketHeader ph) {

		if (_onPongReceivedCallBack) {
			_onPongReceivedCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}

		else
			Skip(ph);
	}

	void IDPipe::OnMtuRequest(PacketHeader ph) {
		DWORD mtu;
		DWORD nBytesRead;
		Read(&mtu, 4, &nBytesRead, NULL);

		if (mtu < _mtu)
			_mtu = mtu;

		SendMtuResponse(_hWritePipe, _mtu);
	}

	void IDPipe::OnMtuResponse(PacketHeader ph) {
		DWORD mtu = 0;
		DWORD nBytesRead;
		Read(&mtu, 4, &nBytesRead, NULL);

		if (mtu < _mtu)
			_mtu = mtu;
	}

	void IDPipe::OnConfigurationReceived(PacketHeader ph) {
		if (_onConfigurationReceivedCallBack) {
			_onConfigurationReceivedCallBack(this, ph);

			if (_nBytesToRead > 0)
				Skip(_nBytesToRead);
		}

		else
			Skip(ph);
	}

	void IDPipe::SendConfiguration(LPVOID lpBuffer, DWORD nBytesCount) {
		Write(DP_SERVICE_CODE_SEND_CONFIGURATION, lpBuffer, nBytesCount);
	}

	void IDPipe::SendMtuRequest(HANDLE& hWriteHandle, DWORD mtu) {
		PacketHeader header(true, 4);
		header.SetServiceCode(DP_SERVICE_CODE_MTU_REQUEST);
		_packetPuilder.PrepareHeader(header);
		_packetPuilder.WriteHeader(hWriteHandle);
		DWORD nBytesWrittten;
		WriteRaw(hWriteHandle, &mtu, 4, &nBytesWrittten);
	}

	void IDPipe::SendMtuResponse(HANDLE& hWriteHandle, DWORD mtu) {
		PacketHeader header(true, 4);
		header.SetServiceCode(DP_SERVICE_CODE_MTU_RESPONSE);
		_packetPuilder.PrepareHeader(header);
		_packetPuilder.WriteHeader(hWriteHandle);
		DWORD nBytesWrittten;
		WriteRaw(hWriteHandle, &mtu, 4, &nBytesWrittten);
	}

	void IDPipe::ServicePacketReceived(PacketHeader header) {

		unsigned int command = header.GetServiceCode();

		switch (command) {
		case DP_SERVICE_CODE_CONNECT:
			OnClientConnect(header);
			break;
		case DP_SERVICE_CODE_DISCONNECT:
			OnOtherSideDisconnect(header);
			break;
		case DP_SERVICE_CODE_TERMINATING:
			OnTerminating(header);
			break;
		case DP_SERVICE_CODE_PING:
			OnPingReceived(header);
			break;
		case DP_SERVICE_CODE_PONG:
			OnPongReceived(header);
			break;
		case DP_SERVICE_CODE_MTU_REQUEST:
			OnMtuRequest(header);
			break;
		case DP_SERVICE_CODE_MTU_RESPONSE:
			OnMtuResponse(header);
			break;
		case DP_SERVICE_CODE_SEND_CONFIGURATION:
			OnConfigurationReceived(header);
			break;
		}
	}

#pragma endregion IDPipe

	bool IDPipe::SendPing(HANDLE& hWriteHandle) {
		PacketHeader header(true, 0);
		header.SetServiceCode(DP_SERVICE_CODE_PING);
		_packetPuilder.PrepareHeader(header);
		return _packetPuilder.WriteHeader(hWriteHandle);
	}

	bool IDPipe::SendPong(HANDLE& hWriteHandle) {
		PacketHeader header(true, 0);
		header.SetServiceCode(DP_SERVICE_CODE_PONG);
		_packetPuilder.PrepareHeader(header);
		return _packetPuilder.WriteHeader(hWriteHandle);
	}

	void IDPipe::ReadFrom(IDPipe* source, DWORD nNumberOfBytesToRead, DWORD nBufferSize) {

		vector<char> buffer(nBufferSize);

		auto cyclecs = nNumberOfBytesToRead / nBufferSize;
		auto rest = nNumberOfBytesToRead % nBufferSize;

		for (DWORD i = 0; i < cyclecs; i++)
		{
			source->Read(buffer.data(), nBufferSize);
			Write(buffer.data(), nBufferSize);
		}

		source->Read(buffer.data(), rest);
		Write(buffer.data(), rest);
	}

	void crdk::dpipes::IDPipe::CopyTo(IDPipe* dest, DWORD nNumberOfBytes, DWORD nBufferSize) {

		vector<char> buffer(nBufferSize);

		auto cyclecs = nNumberOfBytes / nBufferSize;
		auto rest = nNumberOfBytes % nBufferSize;

		for (DWORD i = 0; i < cyclecs; i++)
		{
			Read(buffer.data(), nBufferSize);
			dest->Write(buffer.data(), nBufferSize);
		}

		Read(buffer.data(), rest);
		dest->Write(buffer.data(), rest);
	}

	MemoryDataCallback::MemoryDataCallback(
		void* dataAddress, 
		DWORD dataSize, 
		std::function<void(void* address, DWORD size)> callbackFunction) {
		address = dataAddress;
		size = dataSize;
		callback = callbackFunction;
	}
