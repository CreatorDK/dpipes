#pragma once
#include "dpipebase.h"
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace crdk::dpipes;

//HeapAllocatedData implementation
#pragma region HeapAllocatedData
	HeapAllocatedData::HeapAllocatedData(DWORD size, bool keepAllocatedMemory) {

		if (size > 0)
			_data = new char[size];
		_size = size;
	}

	HeapAllocatedData::~HeapAllocatedData() {
		if (!keepAllocatedMemory && _data != nullptr)
			delete[] _data;
	}

	void* HeapAllocatedData::data() const {
		return _data;
	}

	DWORD HeapAllocatedData::size() const {
		return _size;
	}
#pragma endregion HeapAllocatedData

//IDuplexPipeHandle class implementation
#pragma region IDPipeHandle
	crdk::dpipes::IDPipeHandle::IDPipeHandle() { }
#pragma endregion IDPipeHandle

//IDuplexPipeHandle class implementation
#pragma region IDPipeHandle
	crdk::dpipes::IDPipeHandle::~IDPipeHandle() {}
#pragma endregion IDPipeHandle

//IDuplexPipe base class implementation
#pragma region IDPipe
	crdk::dpipes::IDPipe::IDPipe(const std::wstring& sName, DWORD nInBufferSize, DWORD nOutBufferSize) :
		_sName(sName),
		_nInBufferSize(nInBufferSize),
		_nOutBufferSize(nOutBufferSize) { 
		_skipBuffer = new char[_skipBufferSize];
	}

	crdk::dpipes::IDPipe::~IDPipe() { }

	std::wstring crdk::dpipes::IDPipe::GetName() const {
		return _sName; 
	}

	DWORD crdk::dpipes::IDPipe::BytesToRead() const {
		return _nBytesToRead;
	}

	DWORD crdk::dpipes::IDPipe::GetLastErrorDP() const {
		return _nLastError;
	}

	void crdk::dpipes::IDPipe::Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize) {

		if (_mode == DPIPE_MODE::UNSTARTED)
			return;

		if ((!_bOtherSideDisconnecting && _bIsAlive)) {
			_packetPuilder.PrepareServiceHeader(SERVICE_CODE_DISCONNECT, nConnectDataSize);
			_packetPuilder.WriteHeader(_hWritePipe);

			if (nConnectDataSize) {
				DWORD nBytesWritten;
				WriteRaw(pConnectData, nConnectDataSize, &nBytesWritten);
			}
		}

		if (_tReadThread != nullptr) {
			WaitForSingleObject(_tReadThread, INFINITE);
			CloseHandle(_tReadThread);
			_tReadThread = nullptr;
		}
	}

	void crdk::dpipes::IDPipe::Disconnect() {
		Disconnect(nullptr, 0);
	}

	void crdk::dpipes::IDPipe::SetClientConnectCallback(std::function<void(PacketHeader)> function) {
		_onClientConnectCallBack = function;
	}

	void crdk::dpipes::IDPipe::SetOtherSideDisconnectCallback(std::function<void(PacketHeader)> function) {
		_onPartnerDisconnectCallBack = function;
	}

	void crdk::dpipes::IDPipe::SetPacketHeaderRecevicedCallback(std::function<void(PacketHeader)> function){
		_onPacketHeaderReceivedCallBack = function;
	}

	void crdk::dpipes::IDPipe::Skip(DWORD nBytesToSkip) {

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

	void crdk::dpipes::IDPipe::Skip(PacketHeader header) {
		Skip(header.DataSize());
	}

	HANDLE crdk::dpipes::IDPipe::ReadHandle()
	{
		return _hReadPipe;
	}

	HANDLE crdk::dpipes::IDPipe::WriteHandle()
	{
		return _hWritePipe;
	}

	void crdk::dpipes::IDPipe::OnClientConnect(PacketHeader ph) {
		OnPipeClientConnect();

		_bIsAlive = true;

		if (_onClientConnectCallBack)
			_onClientConnectCallBack(ph);
		else {
			if (ph.DataSize() > 0)
				Skip(ph);
		}
	}

	void crdk::dpipes::IDPipe::OnOtherSideDisconnect(PacketHeader ph) {

		if (_onPartnerDisconnectCallBack)
			_onPartnerDisconnectCallBack(ph);
		else {
			if (ph.DataSize() > 0)
				Skip(ph);
		}

		_bOtherSideDisconnecting = true;
	}

	void crdk::dpipes::IDPipe::OnOtherSideDisconnectPipe() {
		_bIsAlive = false;
		_bListening = false;

		DisconnectPipe();

#ifdef _DEBUG
		//string mode;
		//if (_mode == DPIPE_MODE::CLIENT)
		//	mode = "CLIENT";
		//else
		//	mode = "INNICIATOR";

		//string type;
		//if (_type == DPIPE_TYPE::ANONYMOUS_PIPE)
		//	type = "ANONYMOUS";
		//else
		//	type = "NAMED";

		//cout << mode << " " << type << " Disconnected" << endl;

		//_mode = DPIPE_MODE::UNSTARTED;
#endif //_DEBUG
	}

	void crdk::dpipes::IDPipe::OnTerminating() { }

	void crdk::dpipes::IDPipe::OnPacketHeaderReceived(PacketHeader ph) {
		if (_onPacketHeaderReceivedCallBack)
			_onPacketHeaderReceivedCallBack(ph);
		else {
			if (ph.DataSize() > 0)
				Skip(ph);
		}
	}

	void crdk::dpipes::IDPipe::ServicePacketReceived(PacketHeader ph){

		unsigned int command = PacketBuilder::GetCommand(ph.GetServiceCode());

		switch (command) {
		case SERVICE_CODE_CONNECT:
			OnClientConnect(ph);
			break;
		case SERVICE_CODE_DISCONNECT:
			OnOtherSideDisconnect(ph);
			break;
		case SERVICE_CODE_TERMINATING:
			OnTerminating();
			break;
		}
	}

#pragma endregion IDPipe