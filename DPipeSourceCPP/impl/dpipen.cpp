#pragma once
#include "dpipen.h"
#include <iostream>

using namespace std;
using namespace crdk::dpipes;

#define BufferSize 65536

//Hanlde of named duplex server implementation
#pragma region DPipeNamedHandle

	DPipeNamedHandle::DPipeNamedHandle(const wstring& sServerName, const wstring& sPipeName) {
		_sServerName = sServerName;
		_sPipeName = sPipeName;
	}

	DPipeNamedHandle* DPipeNamedHandle::Create(const std::wstring& sPipeNameFull)
	{
		if (IsNamed(sPipeNameFull)) {
			wstring serverName = GetServerNamePart(sPipeNameFull);
			wstring pipeName = GetNamedPipeNamePart(sPipeNameFull);

			return new DPipeNamedHandle(serverName, pipeName);
		}
			
		else
			throw exception("Invalid DPipeNamed pipe name");
	}

	DPipeNamedHandle::~DPipeNamedHandle() { }

	const wstring DPipeNamedHandle::DefaultPipeName = L"DPipeNamedDefault";

	wstring DPipeNamedHandle::GetServerName() {
		return _sServerName;
	}

	wstring DPipeNamedHandle::GetPipeName() {
		return _sPipeName;
	}

	wstring crdk::dpipes::DPipeNamedHandle::GetMachineName()
	{
		TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD size = sizeof(computerName) / sizeof(computerName[0]);

		if (GetComputerName(computerName, &size)) {
			return wstring(computerName);
		}
		else
			return wstring(L".");
	}

	wstring crdk::dpipes::DPipeNamedHandle::GetNamedPipeNamePart(std::wstring handleString) {
		auto index = handleString.find(L"\\pipe\\");
		return handleString.substr(index + 6);
	}

	wstring crdk::dpipes::DPipeNamedHandle::GetServerNamePart(std::wstring handleString) {
		auto index = handleString.find(L"\\", 2);
		return handleString.substr(2, index - 2);
	}

	bool DPipeNamedHandle::IsNamed(const std::wstring& handleString) {
		
		if (handleString.length() < 10)
			return false;

		if (handleString[0] != '\\' || handleString[1] != '\\')
			return false;

		int pipeWordIndex = 0;

		for (unsigned int i = 2; i < handleString.length(); i++) {
			if (handleString[i] == '\\') {
				pipeWordIndex = i + 1;
				break;
			}
		}

		wstring pipeWord = handleString.substr(pipeWordIndex, 5);

		if (pipeWord != L"pipe\\")
			return false;

		return true;
	}

	wstring DPipeNamedHandle::AsString() const {
		return L"\\\\" + _sServerName + L"\\pipe\\" + _sPipeName;
	}

	DP_TYPE DPipeNamedHandle::GetType() const {
		return NAMED_PIPE;
	}

	wstring DPipeNamed::GetHandleString() {
		return GetHandle()->AsString();
	}

#pragma endregion DPipeNamedHandle

//DuplexPipeNamed class implementation
#pragma region DPipeNamed

	DPipeNamed::DPipeNamed(DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		IDPipe(DPipeNamedHandle::DefaultPipeName, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		_type = DP_TYPE::NAMED_PIPE;
	}

	DPipeNamed::DPipeNamed(const wstring& name,
		DWORD nInBufferSize,
		DWORD nOutBufferSize) :
		IDPipe(name, DWORD_MAX_VALUE, nInBufferSize, nOutBufferSize) {
		_type = DP_TYPE::NAMED_PIPE;
	}

	DPipeNamed::~DPipeNamed() {
		//Disconnect();
	}

	bool DPipeNamed::IsAlive() const {
		return _isAlive;
	}

	DP_MODE DPipeNamed::Mode() const {
		return _mode;
	}

	DP_TYPE DPipeNamed::Type() const
	{
		return _type;
	}

	bool DPipeNamed::Start(LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
		if (_mode == DP_MODE::INNITIATOR)
			throw exception("Pipe already created");
		else if (_mode == DP_MODE::CLIENT)
			throw exception("Cannot create pipe in client mode");

		wstring pipeNameRead = L"\\\\.\\pipe\\" + _sName + L"_read";
		wstring pipeNameWrite = L"\\\\.\\pipe\\" + _sName + L"_write";

		_hReadPipe = CreateNamedPipe(
			&pipeNameRead[0],
			PIPE_ACCESS_DUPLEX ,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			1,
			_nOutBufferSize,
			_nInBufferSize,
			NMPWAIT_USE_DEFAULT_WAIT,
			lpSecurityAttributes
		);

		if (_hReadPipe == INVALID_HANDLE_VALUE)
			throw exception("Unable to create read named pipe");

		_hWritePipe = CreateNamedPipe(
			&pipeNameWrite[0],
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
			1,
			_nOutBufferSize,
			_nInBufferSize,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL
		);

		if (_hWritePipe == INVALID_HANDLE_VALUE)
			throw exception("Unable to create write named pipe");

		_bListening = true;
		_mode = DP_MODE::INNITIATOR;
		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		return true;
	}

	bool DPipeNamed::Start() {
		return Start(nullptr);
	}

	void crdk::dpipes::DPipeNamed::SetUseRemote(bool value) {
		_useRemote = value;
	}

	shared_ptr<IDPipeHandle> DPipeNamed::GetHandle() {
		if (_useRemote) {
			wstring server = DPipeNamedHandle::GetMachineName();
			return shared_ptr<IDPipeHandle>(new DPipeNamedHandle(server, _sName));
		}
		else {
			return shared_ptr<IDPipeHandle>(new DPipeNamedHandle(L".", _sName));
		}
	}

	bool DPipeNamed::ConnectNamed(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD prefix) {
		if (_mode == DP_MODE::CLIENT || _mode == DP_MODE::INNITIATOR)
			return true;

		if (pHandle == nullptr)
			throw new invalid_argument("pHandle is invalid");

		_mode = DP_MODE::CLIENT;

		wstring pipeNameFull = pHandle->AsString();
		_sName = DPipeNamedHandle::GetNamedPipeNamePart(pipeNameFull);

		wstring pipeNameRead = pipeNameFull + L"_read";
		wstring pipeNameWrite = pipeNameFull + L"_write";

		_hReadPipe = CreateFile(
			&pipeNameWrite[0],		// server name 
			GENERIC_READ,			// read  access 
			0,						// no sharing 
			lpSecurityAttributes,					// default security attributes
			OPEN_EXISTING,			// opens existing server 
			0,						// default attributes 
			NULL);					// no template file 

		if (_hReadPipe == INVALID_HANDLE_VALUE)
			throw new invalid_argument("Read pipe handle is invalid");

		if (GetLastError() == ERROR_PIPE_BUSY)
			throw new invalid_argument("Read pipe is busy");

		_hWritePipe = CreateFile(
			&pipeNameRead[0],		// server name 
			GENERIC_WRITE,			// write access 
			0,						// no sharing 
			lpSecurityAttributes,					// default security attributes
			OPEN_EXISTING,			// opens existing server 
			0,						// default attributes 
			NULL);					// no template file 

		_nLastError = GetLastError();

		if (_hReadPipe == INVALID_HANDLE_VALUE)
			throw new invalid_argument("Write pipe handle is invalid");

		if (_nLastError == ERROR_PIPE_BUSY)
			throw new invalid_argument("Write pipe is busy");

		SendMtuRequest(_hWritePipe, _mtu);

		bool listening = true;
		bool success = false;
		auto headerMtu = _packetPuilder.GetPacketHeader(_hReadPipe, listening, success, _nLastError);
		if (!success || headerMtu.GetServiceCode() != DP_SERVICE_CODE_MTU_RESPONSE)
			throw new invalid_argument("Unable to get mtu size");

		_nBytesToRead = 4;
		OnMtuResponse(headerMtu);

		_bListening = true;
		_bIsAlive = true;
		_tReadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadLoop, (LPVOID)this, 0, &_nReadThreadId);

		DWORD numberOfByteWritten;

		PacketHeader header(true, nConnectDataSize);
		header.SetServiceCode(DP_SERVICE_CODE_CONNECT);
		header.SetServicePrefix(prefix);
		_packetPuilder.PrepareHeader(header);
		bool bReturn = WriteInner(_hWritePipe ,_packetPuilder.GetBufferHeaderOut(), _packetPuilder.BufferHeaderSize(), &numberOfByteWritten);

		if (nConnectDataSize) {
			DWORD nBytesWritten;
			bReturn = WriteRaw(pConnectData, nConnectDataSize, &nBytesWritten);
		}

		if (!bReturn)
			_nLastError = GetLastError();

		return bReturn;
	}

	bool DPipeNamed::Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix) {
		return ConnectNamed(pHandle, pConnectData, nConnectDataSize, nullptr, prefix);
	}

	bool DPipeNamed::ConnectNamed(IDPipeHandle* handle, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
		return ConnectNamed(handle, nullptr, 0, lpSecurityAttributes);
	}

	bool DPipeNamed::Connect(IDPipeHandle* handle) {
		return ConnectNamed(handle, nullptr, 0, nullptr);
	}

	bool DPipeNamed::ConnectNamed(std::wstring handleString, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
		return ConnectNamed(handleString, nullptr, 0, lpSecurityAttributes);
	}

	bool DPipeNamed::Connect(wstring handleString) {
		return ConnectNamed(handleString, nullptr, 0, nullptr);
	}

	bool DPipeNamed::ConnectNamed(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
		auto handle = DPipeNamedHandle::Create(handleString);
		bool bResult = ConnectNamed(handle, pConnectData, nConnectDataSize, lpSecurityAttributes);
		delete handle;
		return bResult;
	}

	bool DPipeNamed::Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix) {
		auto handle = DPipeNamedHandle::Create(handleString);
		bool bResult = Connect(handle, pConnectData, nConnectDataSize, prefix);
		delete handle;
		return bResult;
	}

	void DPipeNamed::DisconnectPipe(bool isAlive) {

		if (_mode == DP_MODE::INNITIATOR)
		{
			DisconnectNamedPipe(_hReadPipe);

			if (_hReadPipe != nullptr || _hReadPipe != NULL) {
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}

			DisconnectNamedPipe(_hWritePipe);

			if (_hWritePipe != nullptr || _hWritePipe != NULL) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}
		}

		else if (_mode == DP_MODE::CLIENT) {

			if (_hReadPipe != nullptr || _hReadPipe != NULL) {
				CloseHandle(_hReadPipe);
				_hReadPipe = nullptr;
			}

			if (_hWritePipe != nullptr || _hWritePipe != NULL) {
				CloseHandle(_hWritePipe);
				_hWritePipe = nullptr;
			}
		}
	}

	void crdk::dpipes::DPipeNamed::OnPipeClientConnect() { }

	std::shared_ptr<HeapAllocatedData> DPipeNamed::Read(PacketHeader header) {
		auto heapData = make_shared<HeapAllocatedData>(header.DataSize(), false);
		DWORD nBytesRead;
		Read(heapData->data(), heapData->size(), &nBytesRead, nullptr);
		return heapData;
	}

	void DPipeNamed::ReadLoop(LPVOID lpVoid) {
		auto pInstance = (DPipeNamed*)lpVoid;
		bool disconnection = false;

		if (pInstance->_mode == DP_MODE::INNITIATOR) {

			bool bConnected = ConnectNamedPipe(pInstance->_hReadPipe, NULL);
		
			if (!bConnected) {
				pInstance->_nLastError = GetLastError();
				pInstance->Disconnect();
				return;
			}
		}

		while (pInstance->_bListening) {
			if (pInstance->_nBytesToRead > 0ull) {
				Sleep(1);
				continue;
			}

			bool success;
			PacketHeader header = pInstance->_packetPuilder.GetPacketHeader(pInstance->_hReadPipe, pInstance->_bListening, success, pInstance->_nLastError);
			pInstance->_nBytesRead = 0;
			pInstance->_nBytesToRead = header.DataSize();

			if (header.IsService()) {

				auto command = PacketBuilder::GetCommand(header.GetServiceCode());
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

	bool DPipeNamed::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {

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

	bool DPipeNamed::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead) {
		return Read(lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL);
	}

	bool DPipeNamed::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) {
		DWORD nBytesRead;
		return Read(lpBuffer, nNumberOfBytesToRead, &nBytesRead, NULL);
	}

	bool DPipeNamed::WriteInner(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {

		if (!_bPipeWriteConnected && _mode == DP_MODE::INNITIATOR) {
			DWORD connectionError = 0;
			bool bConnected = ConnectNamedPipe(hWriteHandle, NULL);

			if (!bConnected)
				connectionError = GetLastError();

			if (connectionError == ERROR_PIPE_CONNECTED)
				bConnected = true;

			if (!bConnected) {
				Disconnect();
				return false;
			}
			else
				_bPipeWriteConnected = true;
		}

		char* bufferPtr = (char*)lpBuffer;
		DWORD bytesWritenTotal = 0;

		while (bytesWritenTotal != nNumberOfBytesToWrite) {
			DWORD nBytesToWrite;
			if ((nNumberOfBytesToWrite - bytesWritenTotal) > _mtu)
				nBytesToWrite = _mtu;
			else
				nBytesToWrite = nNumberOfBytesToWrite - bytesWritenTotal;

			DWORD nBytesWritten;
			if (WriteFile(hWriteHandle, bufferPtr, nBytesToWrite, &nBytesWritten, NULL)) {
				bytesWritenTotal += nBytesWritten;
				bufferPtr = bufferPtr + nBytesWritten;
			}
			else {
				*lpNumberOfByteWritten = bytesWritenTotal;
				return false;
			}
		}

		*lpNumberOfByteWritten = bytesWritenTotal;
		return true;
	}

	DPipeNamed* DPipeNamed::Create(const std::wstring& name, DWORD nInBufferSize, DWORD nOutBufferSize) {
		if (DPipeNamedHandle::IsNamed(name))
			return new DPipeNamed(name, nInBufferSize, nOutBufferSize);
		else
			throw exception("Invalid DPipeNamed pipe name");
	}

	bool DPipeNamed::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return Write(DP_SERVICE_CODE_RAW_CLIENT, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeNamed::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return Write(DP_SERVICE_CODE_RAW_CLIENT, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
	}

	bool DPipeNamed::Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		_packetPuilder.PrepareClientHeader(serviceCode, nNumberOfBytesToWrite);
		bool bReturn = WriteInner(_hWritePipe, _packetPuilder.GetBufferHeaderOut(), _packetPuilder.BufferHeaderSize(), lpNumberOfByteWritten);
		bReturn = WriteInner(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
		return bReturn;
	}

	bool DPipeNamed::Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return Write(serviceCode, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
	}

	bool DPipeNamed::WritePacketHeader(PacketHeader header) {
		_packetPuilder.PrepareClientHeader(header.GetCode(), header.DataSize());
		return _packetPuilder.WriteHeader(_hWritePipe);
	}

	bool DPipeNamed::WriteRaw(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return WriteInner(hWriteHandle, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeNamed::WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) {
		return WriteInner(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, lpNumberOfByteWritten);
	}

	bool DPipeNamed::WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
		DWORD nBytesWritten;
		return WriteInner(_hWritePipe, lpBuffer, nNumberOfBytesToWrite, &nBytesWritten);
	}

	IDPipe* DPipeNamed::CreateNewInstance() {
		return new DPipeNamed(_sName, _nInBufferSize, _nOutBufferSize);
	}

#pragma endregion DPipeNamed