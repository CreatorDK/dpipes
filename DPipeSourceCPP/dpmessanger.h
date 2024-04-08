#pragma once
#include <shared_mutex>
#include <boost/numeric/conversion/cast.hpp>
#include "dpipebase.h"

#define DP_REQUEST				0
#define DP_RESPONSE				0
#define DP_INFO_DATA			10
#define DP_INFO_STRING			11
#define DP_WARNING_DATA			20
#define DP_WARNING_STRING		21
#define DP_ERROR_DATA			30
#define DP_ERROR_STRING			31
#define DP_MESSAGE_DATA			40
#define DP_MESSAGE_STRING		41

#define DP_REQUEST_SIZE			24
#define DP_RESPONSE_SIZE		24

#define DP_DATA_BINARY			0
#define DP_DATA_STRING			1
#define DP_DATA_WSTRING			2
#define DP_DATA_STRING_JSON		3
#define DP_DATA_WSRTING_JSON	4

#define DP_HANDLER_NOT_FOUND	404
#define DP_REQUEST_TIMEOUT		408

namespace crdk {
	namespace dpipes {

		class IDPipeMessanger;

		//Struct contains data needs to pass into thread that invoke MessageReciveString events
#pragma region DPMessageReceivedAsyncData
		struct DPMessageReceivedAsyncData {
			IDPipeMessanger* messanger = nullptr;
			PacketHeader header;
			std::shared_ptr<HeapAllocatedData> data;
			DPMessageReceivedAsyncData(IDPipeMessanger* messangerPtr, PacketHeader packetHeader, std::shared_ptr<HeapAllocatedData> heapData);
		};
#pragma endregion DPMessageReceivedAsyncData

		//Struct contains data needs to pass into thread that invoke MessageReciveData events
#pragma region DPMessageStringReceivedAsyncData
		struct DPMessageStringReceivedAsyncData {
			IDPipeMessanger* messanger = nullptr;
			PacketHeader header;
			std::wstring message;
			DPMessageStringReceivedAsyncData(IDPipeMessanger* messangerPtr, PacketHeader packetHeader, std::wstring messageString);
		};
#pragma endregion DPMessageStringReceivedAsyncData

		//Class that contains common methods both uses in DPServer and DPClient classes for exchange messages
#pragma region IDPipeMessanger
		class IDPipeMessanger {
		public:
			IDPipeMessanger(IDPipe* dpipe, DWORD nBufferStringSize);
			~IDPipeMessanger();
			IDPipeMessanger(const IDPipeMessanger&) = delete;
			IDPipeMessanger operator=(const IDPipeMessanger& obj) = delete;
		protected:

			wchar_t* _pBufferString = nullptr;
			DWORD _nBufferStringSize = 0;

			IDPipe* _dpipe = nullptr;
			mutable std::shared_mutex _mutexWritePipe;
			mutable std::shared_mutex _mutexStringBuffer;

			std::function<void(std::wstring)>										_onMessageStringReceived;
			std::function<void(std::shared_ptr<HeapAllocatedData> data)>			_onMessageDataReceived;
			std::function<void(std::wstring)>										_onInfoStringReceived;
			std::function<void(std::shared_ptr<HeapAllocatedData> data)>			_onInfoDataReceived;
			std::function<void(std::wstring)>										_onWarningStringReceived;
			std::function<void(std::shared_ptr<HeapAllocatedData> data)>			_onWarningDataReceived;
			std::function<void(std::wstring)>										_onErrorStringReceived;
			std::function<void(std::shared_ptr<HeapAllocatedData> data)>			_onErrorDataReceived;
		protected:
			void ChekBufferSize(DWORD nBytes);
			std::wstring GetStringFromPipe(DWORD sizeInBytes);

			void OnMessageReceived(PacketHeader& header, std::shared_ptr<HeapAllocatedData> heapData);
			static void OnMessageReceivedAsync(LPVOID dataPtr);
			void OnMessageStringReceived(PacketHeader& header, std::wstring stringMessage);
			static void OnMessageStringReceivedAsync(LPVOID dataPtr);
		public:
			bool keepMessageAllocatedMemory = false;

			bool SendMSG(const std::wstring message);
			bool SendMSG(void* data, DWORD dataSize);
			bool SendInfo(const std::wstring message);
			bool SendInfo(void* data, DWORD dataSize);
			bool SendWarning(const std::wstring message);
			bool SendWarning(void* data, DWORD dataSize);
			bool SendError(const std::wstring message);
			bool SendError(void* data, DWORD dataSize);

			void SetMessageStringReceivedCallback(std::function<void(std::wstring)> function);
			void SetMessageDataReceivedCallback(std::function<void(std::shared_ptr<HeapAllocatedData> data)> function);
			void SetInfoStringReceivedCallback(std::function<void(std::wstring)> function);
			void SetInfoDataReceivedCallback(std::function<void(std::shared_ptr<HeapAllocatedData> data)> function);
			void SetWarningStringReceivedCallback(std::function<void(std::wstring)> function);
			void SetWarningDataReceivedCallback(std::function<void(std::shared_ptr<HeapAllocatedData> data)> function);
			void SetErrorStringReceivedCallback(std::function<void(std::wstring)> function);
			void SetErrorDataReceivedCallback(std::function<void(std::shared_ptr<HeapAllocatedData> data)> function);
		};
#pragma endregion IDPipeMessanger

	}
}