#pragma once
#include <exception>
#include <windows.h>
#include <memory>
#include <boost/cast.hpp>

namespace crdk {
	namespace dpipes {

#define DP_PACKET_HEADER_SIZE 8
#define DP_SERVICE_CODE_CONNECT 1 
#define DP_SERVICE_CODE_DISCONNECT 2 
#define DP_SERVICE_CODE_DISCONNECTED 3
#define DP_SERVICE_CODE_TERMINATING 4
#define DP_SERVICE_CODE_PING 5
#define DP_SERVICE_CODE_PONG 6
#define DP_SERVICE_CODE_MTU_REQUEST 7
#define DP_SERVICE_CODE_MTU_RESPONSE 8
#define DP_SERVICE_CODE_SEND_CONFIGURATION 9
#define DP_SERVICE_CODE_RAW_CLIENT 0 

#define DP_SKIP_BUFFER_SIZE 4096

#define DWORD_MAX_VALUE 4294967295

		//Prototypes
#pragma region prototypes
		class PacketBuilder;
#pragma endregion prototypes

		//Represents header of data packet transferring throw dpipes
#pragma region PacketHeader
		struct PacketHeader {
			PacketHeader(bool isService, DWORD nDataSize);
		private:
			DWORD _code;
			DWORD _dataSize;
			void SetCode(DWORD code);
			PacketHeader(DWORD nDataSize);
		public:
			static PacketHeader Create(DWORD code, DWORD nDataSize);
			bool IsService() const;

			DWORD GetCode() const;

			unsigned char GetServiceCode() const;
			void SetServiceCode(unsigned char code);

			DWORD GetServicePrefix() const;
			void SetServicePrefix(DWORD prefix);

			DWORD GetDataCode() const;
			void SetDataCode(DWORD dataCode);

			DWORD GetDataCodeOnly() const;

			unsigned char GetDataPrefix() const;
			void SetDataPrefix(unsigned char prefix);

			DWORD DataSize() const;
			friend class PacketBuilder;
		};
#pragma region PacketHeader

		//Represents class that build PacketHeader struct for transfering throw dpipes
#pragma region PacketBuilder
		class PacketBuilder {
		public:
			PacketBuilder(DWORD nHeaderSize = DP_PACKET_HEADER_SIZE);
			~PacketBuilder();
		private:
			char* _pBufferHeaderIn;
			char* _pBufferHeaderOut;

			DWORD _nBufferHeaderSize;
		public:
			PacketHeader GetPacketHeader(const HANDLE& pPipe, const bool& bListening, bool& success, DWORD& lastError);
			void PrepareHeader(DWORD serviceCodeRaw, DWORD nDataSize = 0);
			void PrepareHeader(PacketHeader header);
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