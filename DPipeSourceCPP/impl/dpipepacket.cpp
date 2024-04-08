#pragma once
#include "dpipepacket.h"

#include <iomanip>
#include <iostream>

using namespace std;
using namespace crdk::dpipes;

//PacketHeader implementation
#pragma region PacketHeader
	PacketHeader::PacketHeader(DWORD nDataSize, DWORD serviceCode){
		_dataSize = nDataSize;
		_serviceCode = serviceCode;
	}

	bool PacketHeader::IsService() const {
		return _serviceCode & 0x80000000;
	}

	DWORD PacketHeader::GetServiceCode() const {
		return _serviceCode;
	}

	DWORD PacketHeader::GetCommand() const { 
		return _serviceCode & 0x7FFFFFFF;
	}

	DWORD PacketHeader::DataSize() const {
		return _dataSize;
	}
#pragma endregion PacketHeader

//PacketBuilder implementation
#pragma region PacketBuilder
	PacketBuilder::PacketBuilder(DWORD nHeaderSize) {
		_nBufferHeaderSize = nHeaderSize;
		_pBufferHeaderIn = new char[nHeaderSize];
		_pBufferHeaderOut = new char[nHeaderSize];
	}

	PacketBuilder::~PacketBuilder() {
		delete[] _pBufferHeaderIn;
		delete[] _pBufferHeaderOut;
	}

	PacketHeader PacketBuilder::GetPacketHeader(const HANDLE &pPipe, const bool &bListening, DWORD &lastError) {
		DWORD nBytesRead;
		bool bRet = ReadFile(pPipe, _pBufferHeaderIn, _nBufferHeaderSize, &nBytesRead, NULL);

		DWORD dataSize = 0;
		DWORD serviceCode = 0;

		if (!bRet || nBytesRead != _nBufferHeaderSize)
		{
			lastError = GetLastError();

			if (bListening)
				throw exception("Unable to read packet header");
			else {
				dataSize = 0;
				serviceCode = GetServiceCode(SERVICE_CODE_TERMINATING);
			}
		}
		else {
			char* pDataSize = (char*)&dataSize;
			DWORD* pServiceCode = (unsigned long*)&serviceCode;

			memcpy_s(pDataSize, sizeof(dataSize), _pBufferHeaderIn, sizeof(dataSize));
			memcpy_s(pServiceCode, sizeof(serviceCode), _pBufferHeaderIn + sizeof(dataSize), sizeof(serviceCode));
		}

		return PacketHeader(dataSize, serviceCode);
	}

	void PacketBuilder::PrepareHeader(DWORD serviceCodeRaw, DWORD nDataSize) {
		memcpy_s(_pBufferHeaderOut, sizeof(nDataSize), (char*)&nDataSize, sizeof(nDataSize));
		memcpy_s(_pBufferHeaderOut + sizeof(nDataSize), sizeof(serviceCodeRaw), (unsigned char*)&serviceCodeRaw, sizeof(serviceCodeRaw));
	}

	void PacketBuilder::PrepareServiceHeader(DWORD serviceCodeRaw, DWORD nDataSize) {
		serviceCodeRaw = serviceCodeRaw | 0x80000000;
		PrepareHeader(serviceCodeRaw, nDataSize);
	}

	void PacketBuilder::PrepareClientHeader(DWORD serviceCode, DWORD nDataSize) {
		serviceCode = serviceCode & 0x7FFFFFFF;
		PrepareHeader(serviceCode, nDataSize);
	}

	void PacketBuilder::PrepareClientHeader(DWORD nDataSize){
		PrepareClientHeader(SERVICE_CODE_RAW_CLIENT, nDataSize);
	}

	bool PacketBuilder::WriteHeader(const HANDLE& pPipe){
		DWORD nBytesWritten;
		return WriteFile(pPipe, _pBufferHeaderOut, _nBufferHeaderSize, &nBytesWritten, NULL);
	}

	DWORD PacketBuilder::GetCommand(DWORD serviceCode) {
		return serviceCode & 0x7FFFFFFF;
	}

	DWORD PacketBuilder::GetServiceCode(DWORD command) {
		return command | 0x80000000;
	}

	char* PacketBuilder::GetBufferHeaderOut() const {
		return _pBufferHeaderOut;
	}

	DWORD PacketBuilder::BufferHeaderSize() const
	{
		return _nBufferHeaderSize;
	}
#pragma endregion PacketBuilder
