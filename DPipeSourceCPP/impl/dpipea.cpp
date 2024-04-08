#pragma once
#include "dpipea.h"
#include <regex>
#include <iostream>

using namespace std;
using namespace crdk::dpipes;

//DuplexPipeAnonymousHandle class implementation
#pragma region DPipeAnonymousHandle
	DPipeAnonymousHandle::DPipeAnonymousHandle(HANDLE hReadHandle, HANDLE hWriteHandle) :
		_hReadHandle(hReadHandle),
		_hWriteHandle(hWriteHandle) {}


	DPipeAnonymousHandle::DPipeAnonymousHandle(wstring handleString) {
		wstring delimiter = L":::";
		size_t delimeterPos = handleString.find(delimiter);
		wstring hReadStringHandle = handleString.substr(0, delimeterPos);
		wstring hWriteStringHadnle = handleString.substr(delimeterPos + delimiter.length());

		wstringstream ss;
		ss << hex << hReadStringHandle;
		ss >> _hReadHandle;
		ss.clear();
		ss << hex << hWriteStringHadnle;
		ss >> _hWriteHandle;
	}

	DPipeAnonymousHandle* DPipeAnonymousHandle::Start(std::wstring handleString) {
		if (!IsAnonymous(handleString))
			throw exception("Invalid DPipeAnonymous handle string");

		return new DPipeAnonymousHandle(handleString);
	}

	DPipeAnonymousHandle::DPipeAnonymousHandle(wstring hReadHandleString, wstring hWriteHandleString) {
		wstringstream ss;
		ss << hex << hReadHandleString;
		ss >> _hReadHandle;
		ss.clear();
		ss << hex << hWriteHandleString;
		ss >> _hWriteHandle;
	}

	DPipeAnonymousHandle* DPipeAnonymousHandle::Start(std::wstring readHandleString, std::wstring writeHandleString) {
		bool readHandleCorrect = IsHexHandle(readHandleString);

		if (!readHandleCorrect)
			throw exception("Invalid DPipeAnonymous Read Handle");

		bool writeHandleCorrect = IsHexHandle(writeHandleString);

		if (!writeHandleCorrect)
			throw exception("Invalid DPipeAnonymous Write Handle");

		return new DPipeAnonymousHandle(readHandleString, writeHandleString);
	}

	const wstring DPipeAnonymousHandle::DefaultPipeName = L"Anonymous";

	bool DPipeAnonymousHandle::IsAnonymous(const std::wstring handleString) {
		wregex pattern(L"[0-9A-F]{16}:::[0-9A-F]{16}");
		if (!regex_match(handleString, pattern))
			return false;

		return true;
	}

	bool DPipeAnonymousHandle::IsHexHandle(const std::wstring handleString)
	{
		wregex pattern(L"[0-9A-F]{16}");
		if (!regex_match(handleString, pattern))
			return false;

		return true;
	}

	DPipeAnonymousHandle::~DPipeAnonymousHandle() { }

	HANDLE DPipeAnonymousHandle::GetReadHandle() const {
		return _hReadHandle;
	}

	HANDLE DPipeAnonymousHandle::GetWriteHandle() const {
		return _hWriteHandle;
	}

	wstring DPipeAnonymousHandle::AsString() const {
		wostringstream oss;
		oss << _hReadHandle;
		wstring _read_handle_string = oss.str();

		oss.str(L"");
		oss.clear();

		oss << _hWriteHandle;
		wstring _write_handle_string = oss.str();

		wstring result = _read_handle_string + L":::" + _write_handle_string;
		return result;
	}

	DPIPE_TYPE DPipeAnonymousHandle::GetType() const {
		return ANONYMOUS_PIPE;
	}
#pragma endregion DPipeAnonymousHandle

//DuplexPipeAnonymous class implementation
#pragma region DPipeAnonymous

	DPipeAnonymous::DPipeAnonymous(DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		IDPipe(DPipeAnonymousHandle::DefaultPipeName, nInBufferSize, nOutBufferSize) { 
		_type = DPIPE_TYPE::ANONYMOUS_PIPE;
	}

	DPipeAnonymous::DPipeAnonymous(const wstring& sName,
		DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		IDPipe(sName, nInBufferSize, nOutBufferSize) { 
		_type = DPIPE_TYPE::ANONYMOUS_PIPE;
	}

	bool DPipeAnonymous::IsAlive() const {
		return _bIsAlive;
	}

	DPIPE_MODE DPipeAnonymous::Mode() const {
		return _mode;
	}

	DPIPE_TYPE DPipeAnonymous::Type() const {
		return _type;
	}

	bool DPipeAnonymous::Start() {
		if (_mode == DPIPE_MODE::INNITIATOR)
			throw exception("Pipe already created");
		else if (_mode == DPIPE_MODE::CLIENT)
			throw exception("Cannot create pipe in client mode");

		bool bReturn = false;

		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

		bReturn = CreatePipe(&_hReadPipe, &_hWritePipeClient, &sa, _nInBufferSize);
		SetHandleInformation(_hReadPipe, HANDLE_FLAG_INHERIT, 0);

		if (!bReturn)
			throw exception("Unable to create read pipe");

		bReturn = CreatePipe(&_hReadPipeClient, &_hWritePipe, &sa, _nInBufferSize);
		SetHandleInformation(_hWritePipe, HANDLE_FLAG_INHERIT, 0);

		if (!bReturn) {
			CloseHandle(_hReadPipe);
			throw exception("Unable to create write pipe");
		}

		_bListening = true;
		_mode = DPIPE_MODE::INNITIATOR;

		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		return bReturn;
	}

	shared_ptr<IDPipeHandle> DPipeAnonymous::GetHandle() {

		if (_mode == DPIPE_MODE::UNSTARTED)
			throw exception("Unable to get handle of unstarded dpipe");

		if (_mode == DPIPE_MODE::CLIENT)
			throw exception("Unable to get handle in client mode");

		if (_bIsAlive)
			return nullptr;

		return shared_ptr<IDPipeHandle>(new DPipeAnonymousHandle(_hReadPipeClient, _hWritePipeClient));
	}

	wstring DPipeAnonymous::GetHandleString() {
		return GetHandle()->AsString();
	}

	bool DPipeAnonymous::Connect(IDPipeHandle *pHandle) {
		if (_mode == DPIPE_MODE::CLIENT || _mode == DPIPE_MODE::INNITIATOR)
			return true;

		if (pHandle == nullptr)
			throw new invalid_argument("Handle is invalid");
		
		auto pDPipeAnonymousHandler = DPipeAnonymousHandle::Start(pHandle->AsString());
		bool bResult = Connect(pDPipeAnonymousHandler);
		delete pDPipeAnonymousHandler;
		return bResult;
	}

	bool DPipeAnonymous::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize)
	{
		auto pDPipeAnonymousHandler = DPipeAnonymousHandle::Start(pHandle->AsString());
		bool bResult = Connect(pDPipeAnonymousHandler, pConnectData, nConnectDataSize);
		delete pDPipeAnonymousHandler;
		return bResult;
	}

	bool DPipeAnonymous::Connect(DPipeAnonymousHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize)
	{
		if (_mode == DPIPE_MODE::CLIENT || _mode == DPIPE_MODE::INNITIATOR)
			return true;

		if (pHandle == nullptr)
			throw new invalid_argument("pHandle is invalid");

		_hReadPipe = pHandle->GetReadHandle();
		_hWritePipe = pHandle->GetWriteHandle();

		_mode = DPIPE_MODE::CLIENT;
		_bListening = true;
		_bIsAlive = true;
		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		_packetPuilder.PrepareServiceHeader(SERVICE_CODE_CONNECT, nConnectDataSize);
		bool bReturn = _packetPuilder.WriteHeader(_hWritePipe);
		DWORD nBytesWritten;

		if (nConnectDataSize > 0) {
			bReturn = WriteRaw(pConnectData, nConnectDataSize, &nBytesWritten);
		}

		if (!bReturn)
			_nLastError = GetLastError();

		return bReturn;
	}

	bool DPipeAnonymous::Connect(DPipeAnonymousHandle* handle) {
		return Connect(handle, nullptr, 0);
	}

	bool DPipeAnonymous::Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) {
		auto handle = DPipeAnonymousHandle::Start(handleString);
		bool bResult = Connect(handle, pConnectData, nConnectDataSize);
		delete handle;
		return bResult;
	}

	bool DPipeAnonymous::Connect(wstring handleString) {
		return Connect(handleString, nullptr, 0);
	}

	void DPipeAnonymous::DisconnectPipe() {

		if (_mode == DPIPE_MODE::INNITIATOR) {

			if (_hWritePipe != nullptr) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}

			if (_hReadPipe != nullptr) {
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}
		}

		else if (_mode == DPIPE_MODE::CLIENT) {

			if (_hWritePipe != nullptr) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}

			if (_hReadPipe != nullptr) {
				//Here thread blocikng (client cannot close handle) until Inniciator close his write end of pipe
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}
		}
	}

	bool DPipeAnonymous::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
		bool bReturn = FALSE;
		bReturn = ReadFile(_hReadPipe, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
		_nBytesRead += (*lpNumberOfBytesRead);
		_nBytesToRead -= (*lpNumberOfBytesRead);

		return bReturn;
	}

	bool DPipeAnonymous::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return Write(SERVICE_CODE_RAW_CLIENT, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeAnonymous::Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		_packetPuilder.PrepareClientHeader(serviceCode, nNumberOfBytesToWrite);
		bool bReturn = FALSE;
		bReturn = _packetPuilder.WriteHeader(_hWritePipe);
		bReturn = WriteRaw(lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
		return bReturn;
	}

	bool DPipeAnonymous::WritePacketHeader(PacketHeader header) {
		_packetPuilder.PrepareClientHeader(header.GetServiceCode(), header.DataSize());
		return _packetPuilder.WriteHeader(_hWritePipe);
	}

	bool DPipeAnonymous::WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return WriteFile(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten, NULL);
	}

	void DPipeAnonymous::ReadLoop(LPVOID data) {
		auto pInstance = (DPipeAnonymous*)data;
		bool disconnection = false;

		while (pInstance->_bListening)
		{
			if (pInstance->_nBytesToRead > 0ull) {
				Sleep(1);
				continue;
			}

			PacketHeader header = pInstance->_packetPuilder.GetPacketHeader(pInstance->_hReadPipe, pInstance->_bListening, pInstance->_nLastError);
			pInstance->_nBytesToRead = header.DataSize();
			auto command = PacketBuilder::GetCommand(header.GetServiceCode());

			if (header.IsService()) {
				pInstance->ServicePacketReceived(header);
				if (command == SERVICE_CODE_DISCONNECTED) {
					disconnection = true;
					break;
				}
				else if (command == SERVICE_CODE_DISCONNECT) {
					pInstance->_packetPuilder.PrepareServiceHeader(SERVICE_CODE_DISCONNECTED);
					pInstance->_packetPuilder.WriteHeader(pInstance->_hWritePipe);
					disconnection = true;
					break;
				}
			}
			else {
				pInstance->OnPacketHeaderReceived(header);
			}
		}

		if (disconnection)
			pInstance->OnOtherSideDisconnectPipe();
	}

	void crdk::dpipes::DPipeAnonymous::OnPipeClientConnect() {
		CloseHandle(_hReadPipeClient);
		_hReadPipeClient = nullptr;
		CloseHandle(_hWritePipeClient);
		_hWritePipeClient = nullptr;
	}

	DPipeAnonymous::~DPipeAnonymous() {
		Disconnect();
	}

#pragma endregion DPipeAnonymous