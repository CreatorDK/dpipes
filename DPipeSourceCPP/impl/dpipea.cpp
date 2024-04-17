#pragma once
#include "dpipea.h"
#include <regex>
#include <iostream>
#include <io.h>
#include <fcntl.h>

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

	DPipeAnonymousHandle* DPipeAnonymousHandle::Create(std::wstring handleString) {
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

	DPipeAnonymousHandle* DPipeAnonymousHandle::Create(std::wstring readHandleString, std::wstring writeHandleString) {
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
		wregex pattern1(L"[0-9A-F]{16}:::[0-9A-F]{16}");
		wregex pattern2(L"[0-9A-F]{8}:::[0-9A-F]{8}");

		if (regex_match(handleString, pattern1) || regex_match(handleString, pattern2))
			return true;
		else
			return false;
	}

	bool DPipeAnonymousHandle::IsHexHandle(const std::wstring handleString)
	{
		wregex pattern1(L"[0-9A-F]{16}");
		wregex pattern2(L"[0-9A-F]{8}");

		if (regex_match(handleString, pattern1) || regex_match(handleString, pattern2))
			return true;
		else
			return false;
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

	DP_TYPE DPipeAnonymousHandle::GetType() const {
		return ANONYMOUS_PIPE;
	}
#pragma endregion DPipeAnonymousHandle

//DuplexPipeAnonymous class implementation
#pragma region DPipeAnonymous

	DPipeAnonymous::DPipeAnonymous(DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		//IDPipe(DPipeAnonymousHandle::DefaultPipeName, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		IDPipe(DPipeAnonymousHandle::DefaultPipeName, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		_type = DP_TYPE::ANONYMOUS_PIPE;
	}

	DPipeAnonymous::DPipeAnonymous(const wstring& sName,
		DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		//IDPipe(sName, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		IDPipe(sName, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		_type = DP_TYPE::ANONYMOUS_PIPE;
	}

	DPipeAnonymous::~DPipeAnonymous() {
		//Disconnect();
	}

	bool DPipeAnonymous::IsAlive() const {
		return _bIsAlive;
	}

	DP_MODE DPipeAnonymous::Mode() const {
		return _mode;
	}

	DP_TYPE DPipeAnonymous::Type() const {
		return _type;
	}

	bool DPipeAnonymous::Start() {
		if (_mode == DP_MODE::INNITIATOR)
			throw exception("Pipe already created");
		else if (_mode == DP_MODE::CLIENT)
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
		_mode = DP_MODE::INNITIATOR;

		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		return bReturn;
	}

	shared_ptr<IDPipeHandle> DPipeAnonymous::GetHandle() {

		if (_mode == DP_MODE::UNSTARTED)
			throw exception("Unable to get handle of unstarded dpipe");

		if (_mode == DP_MODE::CLIENT)
			throw exception("Unable to get handle in client mode");

		if (_bIsAlive)
			return nullptr;

		return shared_ptr<IDPipeHandle>(new DPipeAnonymousHandle(_hReadPipeClient, _hWritePipeClient));
	}

	wstring DPipeAnonymous::GetHandleString() {
		return GetHandle()->AsString();
	}

	bool DPipeAnonymous::Connect(IDPipeHandle *pHandle) {
		if (_mode == DP_MODE::CLIENT || _mode == DP_MODE::INNITIATOR)
			return true;

		if (pHandle == nullptr)
			throw new invalid_argument("Handle is invalid");
		
		auto pDPipeAnonymousHandler = DPipeAnonymousHandle::Create(pHandle->AsString());
		bool bResult = ConnectAnonymus(pDPipeAnonymousHandler);
		delete pDPipeAnonymousHandler;
		return bResult;
	}

	bool DPipeAnonymous::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix)
	{
		auto pDPipeAnonymousHandler = DPipeAnonymousHandle::Create(pHandle->AsString());
		bool bResult = ConnectAnonymus(pDPipeAnonymousHandler, pConnectData, nConnectDataSize, prefix);
		delete pDPipeAnonymousHandler;
		return bResult;
	}

	bool DPipeAnonymous::ConnectAnonymus(DPipeAnonymousHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix)
	{
		if (_mode == DP_MODE::CLIENT || _mode == DP_MODE::INNITIATOR)
			return true;

		if (pHandle == nullptr)
			throw new invalid_argument("pHandle is invalid");

		HANDLE readPipe = pHandle->GetReadHandle();

		if (readPipe == INVALID_HANDLE_VALUE)
			throw new invalid_argument("Read Handle is invalid");

		HANDLE writePipe = pHandle->GetWriteHandle();

		if (writePipe == INVALID_HANDLE_VALUE)
			throw new invalid_argument("Write Handle is invalid");

		if (!SendPing(writePipe))
			throw new invalid_argument("Unable to write to the pipe");

		bool listening = true;
		bool success = false;
		auto headerPong = _packetPuilder.GetPacketHeader(readPipe, listening, success, _nLastError);

		if (!success || headerPong.GetServiceCode() != DP_SERVICE_CODE_PONG)
			throw new invalid_argument("Unable to read from the pipe");

		_hReadPipe = pHandle->GetReadHandle();
		_hWritePipe = pHandle->GetWriteHandle();

		SendMtuRequest(_hWritePipe, _mtu);

		auto headerMtu = _packetPuilder.GetPacketHeader(_hReadPipe, listening, success, _nLastError);
		if (!success || headerMtu.GetServiceCode() != DP_SERVICE_CODE_MTU_RESPONSE)
			throw new invalid_argument("Unable to get mtu size");

		_nBytesToRead = 4;
		OnMtuResponse(headerMtu);

		_mode = DP_MODE::CLIENT;
		_bListening = true;
		_bIsAlive = true;
		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		PacketHeader header(true, nConnectDataSize);
		header.SetServiceCode(DP_SERVICE_CODE_CONNECT);
		header.SetServicePrefix(prefix);
		_packetPuilder.PrepareHeader(header);
		bool bReturn = _packetPuilder.WriteHeader(_hWritePipe);
		DWORD nBytesWritten;

		if (nConnectDataSize > 0) {
			bReturn = WriteRaw(pConnectData, nConnectDataSize, &nBytesWritten);
		}

		if (!bReturn)
			_nLastError = GetLastError();

		return bReturn;
	}

	bool DPipeAnonymous::ConnectAnonymus(DPipeAnonymousHandle* handle) {
		return Connect(handle, nullptr, 0);
	}

	bool DPipeAnonymous::Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix) {
		auto handle = DPipeAnonymousHandle::Create(handleString);
		bool bResult = ConnectAnonymus(handle, pConnectData, nConnectDataSize, prefix);
		delete handle;
		return bResult;
	}

	bool DPipeAnonymous::Connect(wstring handleString) {
		return Connect(handleString, nullptr, 0);
	}

	void DPipeAnonymous::DisconnectPipe(bool isAlive) {

		if (_mode == DP_MODE::INNITIATOR) {

			//CloseHandle(_hReadPipeClient);
			//_hReadPipeClient = nullptr;

			//CloseHandle(_hWritePipeClient);
			//_hWritePipeClient = nullptr;

			if (_hWritePipe != nullptr || _hWritePipe != NULL) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}

			if (_hReadPipe != nullptr || _hReadPipe != NULL) {
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}
		}

		else if (_mode == DP_MODE::CLIENT) {

			if (_hWritePipe != nullptr || _hWritePipe != NULL) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}

			if (_hReadPipe != nullptr || _hReadPipe != NULL) {
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}
		}
	}

	bool DPipeAnonymous::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {

		char* bufferPtr = (char*)lpBuffer;
		DWORD bytesReadTotal = 0;

		while (nNumberOfBytesToRead != bytesReadTotal) {
			DWORD nBytesToRead;
			if ((nNumberOfBytesToRead - bytesReadTotal) > _mtu)
				nBytesToRead = _mtu;
			else
				nBytesToRead = nNumberOfBytesToRead - bytesReadTotal;

			DWORD nBytesRead = 0;

			if (ReadFile(_hReadPipe, bufferPtr, nBytesToRead, &nBytesRead, lpOverlapped)) {
				bufferPtr = bufferPtr + nBytesRead;
				bytesReadTotal += nBytesRead;
				_nBytesRead += nBytesRead;
				_nBytesToRead -= nBytesRead;
			}
			else {
				*lpNumberOfBytesRead = bytesReadTotal;
				return false;
			}
		}
		
		*lpNumberOfBytesRead = bytesReadTotal;
		return true;
	}

	bool DPipeAnonymous::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead) {
		return Read(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
	}

	bool DPipeAnonymous::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) {
		DWORD nBytesRead;
		return Read(lpBuffer, nNumberOfBytesToRead, &nBytesRead, NULL);
	}

	shared_ptr<HeapAllocatedData> DPipeAnonymous::Read(PacketHeader header) {
		auto heapData = make_shared<HeapAllocatedData>(header.DataSize(), false);
		DWORD nBytesRead;
		Read(heapData->data(), heapData->size(), &nBytesRead, nullptr);
		return heapData;
	}

	bool DPipeAnonymous::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return Write(DP_SERVICE_CODE_RAW_CLIENT, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeAnonymous::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return Write(DP_SERVICE_CODE_RAW_CLIENT, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
	}

	bool DPipeAnonymous::Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		_packetPuilder.PrepareClientHeader(serviceCode, nNumberOfBytesToWrite);
		bool bReturn = FALSE;
		bReturn = _packetPuilder.WriteHeader(_hWritePipe);
		bReturn = WriteRaw(lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
		return bReturn;
	}

	bool crdk::dpipes::DPipeAnonymous::Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return Write(serviceCode, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
	}

	bool DPipeAnonymous::WritePacketHeader(PacketHeader header) {
		_packetPuilder.PrepareClientHeader(header.GetCode(), header.DataSize());
		return _packetPuilder.WriteHeader(_hWritePipe);
	}

	bool DPipeAnonymous::WriteRaw(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {

		char* bufferPtr = (char*)lpBuffer;
		DWORD bytesWrittenTotal = 0;

		while (bytesWrittenTotal != nNumberOfBytesToWrite) {
			DWORD nBytesToWrite;
			if ((nNumberOfBytesToWrite - bytesWrittenTotal) > _mtu)
				nBytesToWrite = _mtu;
			else
				nBytesToWrite = nNumberOfBytesToWrite - bytesWrittenTotal;

			DWORD nBytesWritten;
			if (WriteFile(hWriteHandle, bufferPtr, nBytesToWrite, &nBytesWritten, NULL)) {
				bytesWrittenTotal += nBytesWritten;
				bufferPtr = bufferPtr + nBytesWritten;
			}
			else {
				*lpNumberOfByteWritten = bytesWrittenTotal;
				return false;
			}
		}

		*lpNumberOfByteWritten = bytesWrittenTotal;
		return true;
	}

	bool DPipeAnonymous::WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return WriteRaw(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeAnonymous::WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return WriteRaw(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
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

			bool sucess;
			PacketHeader header = pInstance->_packetPuilder.GetPacketHeader(pInstance->_hReadPipe, pInstance->_bListening, sucess, pInstance->_nLastError);
			pInstance->_nBytesRead = 0;
			pInstance->_nBytesToRead = header.DataSize();

			if (header.IsService()) {

				auto command = header.GetServiceCode();
				pInstance->ServicePacketReceived(header);

				if (command == DP_SERVICE_CODE_DISCONNECTED) {
					disconnection = true;
					break;
				}
				else if (command == DP_SERVICE_CODE_DISCONNECT) {
					if (pInstance->_clientEmulating)
						Sleep(100);

					pInstance->_packetPuilder.PrepareServiceHeader(DP_SERVICE_CODE_DISCONNECTED);
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

		//CloseHandle(_hReadPipeClient);
		//_hReadPipeClient = nullptr;

		//CloseHandle(_hWritePipeClient);
		//_hWritePipeClient = nullptr;
	}

	IDPipe*  DPipeAnonymous::CreateNewInstance() {
		return new DPipeAnonymous(_nInBufferSize, _nOutBufferSize);
	}

#pragma endregion DPipeAnonymous