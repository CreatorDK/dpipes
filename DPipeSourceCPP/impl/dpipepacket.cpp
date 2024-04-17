#pragma once
#include "dpipepacket.h"
#include <iomanip>
#include <iostream>

using namespace std;
using namespace crdk::dpipes;

//PacketHeader implementation
#pragma region PacketHeader
	PacketHeader::PacketHeader(bool isService, DWORD nDataSize){
		if (isService)
			_code = 0x80000000;
		else
			_code = 0;

		_dataSize = nDataSize;
	}

	PacketHeader::PacketHeader(DWORD nDataSize) {
		_code = 0;
		_dataSize = nDataSize;
	}

	DWORD PacketHeader::GetCode() const {
		return _code;
	}

	void PacketHeader::SetCode(DWORD code) {
		_code = code;
	}

	PacketHeader PacketHeader::Create(DWORD code, DWORD nDataSize) {
		PacketHeader header(nDataSize);
		header.SetCode(code);
		return header;
	}

	bool PacketHeader::IsService() const {
		return _code & 0x80000000;
	}

	unsigned char PacketHeader::GetServiceCode() const {

		if (!IsService())
			throw exception("Unable to get service code from non-service packet");
		
		//Remove unused digits
		DWORD mask = 0xFF;
		DWORD codeLong = _code & mask;

		return boost::numeric_cast<unsigned char>(codeLong);
	}

	void PacketHeader::SetServiceCode(unsigned char code) {

		if (!IsService())
			throw exception("Unable to set service code on non-service packet");

		DWORD codeLong = (DWORD)code;

		_code = _code | codeLong;
	}

	DWORD PacketHeader::GetServicePrefix() const {

		if (!IsService())
			throw exception("Unable to get service prefix from non-service packet");

		//Remove significant bit
		DWORD result = _code & 0x7FFFFFFF;
		return result >> 8;
	}

	void PacketHeader::SetServicePrefix(DWORD prefix) {

		if (!IsService())
			throw exception("Unable to set service prefix from non-service packet");

		if (prefix > 16777215)
			throw exception("Illegal prefix value. Prefix value range is 0..16777215");

		prefix = prefix << 23;
		_code = _code | prefix;
	}

	DWORD PacketHeader::GetDataCode() const {

		if (IsService())
			throw exception("Unable to get data code from service packet");

		//Remove significant bit
		return _code & 0x7FFFFFFF;
	}

	DWORD PacketHeader::GetDataCodeOnly() const {

		if (IsService())
			throw exception("Unable to get data code from service packet");

		//Remove first 8 bits
		return _code & 0xFFFFFF;
	}

	void crdk::dpipes::PacketHeader::SetDataCode(DWORD dataCode) {

		if (IsService())
			throw exception("Unable to set data code on service packet");

		if (dataCode > 2147483648)
			throw exception("Illegal prefix value. Prefix value range is 0..2147483648");

		_code = _code | dataCode;
	}

	unsigned char crdk::dpipes::PacketHeader::GetDataPrefix() const {
		
		if (IsService())
			throw exception("Unable to get data prefix from service packet");

		//Remove significant bit
		DWORD result = _code & 2147483647;

		result = result >> 24;

		return boost::numeric_cast<unsigned char>(result);
	}

	void crdk::dpipes::PacketHeader::SetDataPrefix(unsigned char prefix) {

		if (IsService())
			throw exception("Unable to set data prefix on service packet");

		if (prefix > 127)
			throw exception("Illegal prefix value. Prefix value range is 0..127");

		DWORD prefixLong = (DWORD)prefix;
		prefixLong = prefixLong << 24;

		_code = _code | prefixLong;
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

	PacketHeader PacketBuilder::GetPacketHeader(const HANDLE &pPipe, const bool &bListening, bool &success, DWORD &lastError) {
		DWORD nBytesRead;
		bool bRet = ReadFile(pPipe, _pBufferHeaderIn, _nBufferHeaderSize, &nBytesRead, NULL);

		DWORD dataSize = 0;
		DWORD serviceCode = 0;

		if (!bRet || nBytesRead != _nBufferHeaderSize)
		{
			lastError = GetLastError();

			if (bListening)
				success = false;

			else {
				dataSize = 0;
				serviceCode = GetServiceCode(DP_SERVICE_CODE_TERMINATING);
				success = true;
			}
		}
		else {
			char* pDataSize = (char*)&dataSize;
			DWORD* pServiceCode = (unsigned long*)&serviceCode;

			memcpy_s(pDataSize, sizeof(dataSize), _pBufferHeaderIn, sizeof(dataSize));
			memcpy_s(pServiceCode, sizeof(serviceCode), _pBufferHeaderIn + sizeof(dataSize), sizeof(serviceCode));
			success = true;
		}

		PacketHeader header = PacketHeader::Create(serviceCode, dataSize);
		return header;
	}

	void PacketBuilder::PrepareHeader(DWORD serviceCodeRaw, DWORD nDataSize) {
		memcpy_s(_pBufferHeaderOut, sizeof(nDataSize), (char*)&nDataSize, sizeof(nDataSize));
		memcpy_s(_pBufferHeaderOut + sizeof(nDataSize), sizeof(serviceCodeRaw), (unsigned char*)&serviceCodeRaw, sizeof(serviceCodeRaw));
	}

	void PacketBuilder::PrepareHeader(PacketHeader header) {
		PrepareHeader(header.GetCode(), header.DataSize());
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
		PrepareClientHeader(DP_SERVICE_CODE_RAW_CLIENT, nDataSize);
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
