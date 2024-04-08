#pragma once
#include <exception>
#include <windows.h>
#include <memory>

namespace crdk {
	namespace dpipes {

#define PACKET_HEADER_SIZE 8
#define SERVICE_CODE_CONNECT 1 
#define SERVICE_CODE_DISCONNECT 2 
#define SERVICE_CODE_DISCONNECTED 3
#define SERVICE_CODE_RAW_CLIENT 0 
#define SERVICE_CODE_TERMINATING 4294967295
#define SKIP_BUFFER_SIZE 4096

		//Prototypes
#pragma region prototypes
		class PacketBuilder;
#pragma endregion prototypes

		//Represents header of data packet transferring throw dpipes
#pragma region PacketHeader
		struct PacketHeader {
			PacketHeader(DWORD nDtaSize, DWORD serviceCode);
		private:
			DWORD _serviceCode;
			DWORD _dataSize;
		public:
			bool IsService() const;
			DWORD GetServiceCode() const;
			DWORD GetCommand() const;
			DWORD DataSize() const;
			friend class PacketBuilder;
		};
#pragma region PacketHeader

		//Represents class that build PacketHeader struct for transfering throw dpipes
#pragma region PacketBuilder
		class PacketBuilder {
		public:
			PacketBuilder(DWORD nHeaderSize = PACKET_HEADER_SIZE);
			~PacketBuilder();
		private:
			char* _pBufferHeaderIn;
			char* _pBufferHeaderOut;

			DWORD _nBufferHeaderSize;
		public:
			PacketHeader GetPacketHeader(const HANDLE& pPipe, const bool& bListening, DWORD& lastError);
			void PrepareHeader(DWORD serviceCodeRaw, DWORD nDataSize = 0);
			void PrepareServiceHeader(DWORD serviceCodeRaw, DWORD nDataSize = 0);
			void PrepareClientHeader(DWORD serviceCode, DWORD nDataSize = 0);
			void PrepareClientHeader(DWORD nDataSize = 0);
			bool WriteHeader(const HANDLE& pPipe);
			static DWORD GetCommand(DWORD serviceCode);
			static DWORD GetServiceCode(DWORD comand);
			char* GetBufferHeaderOut() const;
			DWORD BufferHeaderSize() const;
		};
#pragma endregion PacketBuilder

	}
}