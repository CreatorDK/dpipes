#pragma once
#include <shared_mutex>
#include <boost/numeric/conversion/cast.hpp>
#include "dpipebase.h"

namespace crdk {
	namespace dpipes {

		class IDPipeMessanger;

#pragma region DP_STRING_TYPE
		enum DP_STRING_TYPE {
			STRING = 0,
			WSTRING = 1,
			UNDEFINED = 2
		};
#pragma endregion DP_STRING_TYPE

		//DPipe message type enumeration
#pragma region DP_MESSAGE_TYPE
		enum DP_MESSAGE_TYPE {
			MESSAGE = 0,
			MESSAGE_INFO = 1,
			MESSAGE_WARNING = 2,
			MESSAGE_ERROR = 3
		};
#pragma endregion DP_MESSAGE_TYPE

		//Struct contains data needs to pass into thread that invoke MessageReciveString events
#pragma region DPMessageReceivedAsyncData
		struct DPMessageReceivedAsyncData {
			IDPipeMessanger* messanger = nullptr;
			PacketHeader header;
			std::shared_ptr<HeapAllocatedData> data;
			IDPipe* pipe;
			DPMessageReceivedAsyncData(IDPipeMessanger* messangerPtr, PacketHeader packetHeader, std::shared_ptr<HeapAllocatedData> heapData, IDPipe* dpipe);
		};
#pragma endregion DPMessageReceivedAsyncData

		//Struct contains data that will be sent asynchronously and callback function
#pragma region SendDataAsyncContainer
		struct SendDataAsyncContainer {
			IDPipeMessanger* dpMessanger = nullptr;
			DP_MESSAGE_TYPE type = DP_MESSAGE_TYPE::MESSAGE;
			void* data;
			DWORD size;
			std::function<void(IDPipe*)> callback;
			SendDataAsyncContainer(IDPipeMessanger* messangerPtr, DP_MESSAGE_TYPE messageType, void* dataPtr, DWORD dataSize, std::function<void(IDPipe*)> callbackFunction);
		};
#pragma endregion SendDataAsyncContainer

		//Struct contains data that will be sent asynchronously and callback function
#pragma region SendStringAsyncContainer
		struct SendStringAsyncContainer {
			IDPipeMessanger* dpMessanger = nullptr;
			DP_MESSAGE_TYPE type = DP_MESSAGE_TYPE::MESSAGE;
			std::string data;
			std::function<void(IDPipe*)> callback;
			SendStringAsyncContainer(IDPipeMessanger* messangerPtr, DP_MESSAGE_TYPE messageType, std::string dataStr, std::function<void(IDPipe*)> callbackFunction);
		};
#pragma endregion SendStringAsyncContainer

		//Struct contains data that will be sent asynchronously and callback function
#pragma region SendWStringAsyncContainer
		struct SendWStringAsyncContainer {
			IDPipeMessanger* dpMessanger = nullptr;
			DP_MESSAGE_TYPE type = DP_MESSAGE_TYPE::MESSAGE;
			std::wstring data;
			std::function<void(IDPipe*)> callback;
			SendWStringAsyncContainer(IDPipeMessanger* messangerPtr, DP_MESSAGE_TYPE messageType, std::wstring dataStr, std::function<void(IDPipe*)> callbackFunction);
		};
#pragma endregion SendWStringAsyncContainer

		//Class that contains common methods both uses in DPServer and DPClient classes for exchange messages
#pragma region IDPipeMessanger
		class IDPipeMessanger {
		public:
			IDPipeMessanger(IDPipe* dpipe);
			~IDPipeMessanger();
			IDPipeMessanger(const IDPipeMessanger&) = delete;
			IDPipeMessanger operator=(const IDPipeMessanger& obj) = delete;
		protected:
			IDPipe* _dpipe = nullptr;
			mutable std::shared_mutex _mutexWrite;
			mutable std::shared_mutex _mutexRead;

			DWORD _stringEncodingCode = DP_ENCODING_UTF8;
			DWORD _wstringEncodingCode = DP_ENCODING_UNICODE;

			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onMessageStringReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onMessageDataReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onInfoStringReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onInfoDataReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onWarningStringReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onWarningDataReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onErrorStringReceived;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>			_onErrorDataReceived;
		protected:
			void OnMessageReceived(IDPipe* pipe, PacketHeader& header, std::shared_ptr<HeapAllocatedData> heapData);
			static void OnMessageReceivedAsync(LPVOID dataPtr);

			void OnMessageStringReceived(IDPipe* pipe, PacketHeader& header, std::shared_ptr<HeapAllocatedData> heapData);
			static void OnMessageStringReceivedAsync(LPVOID dataPtr);
		public:
			static DWORD AddEncodingCode(DWORD serviceCode, DWORD encodingCode);
			static DWORD GetEncodingCode(PacketHeader header);
			static DWORD GetEncodingCode(DP_STRING_TYPE stringType);
			static DP_STRING_TYPE GetStringType(DWORD encodingCode);

			static DP_STRING_TYPE GetStringType(PacketHeader header);

			static std::wstring StringToWString(const std::string& str);
			static std::string WStringToString(const std::wstring& wstr);

			std::string GetString(PacketHeader header, std::shared_ptr<HeapAllocatedData> heapData);
			std::string GetString(PacketHeader header);

			static std::string GetString(std::shared_ptr<HeapAllocatedData> heapData, int encodingCode);

			std::wstring GetWString(PacketHeader header, std::shared_ptr<HeapAllocatedData> heapData);
			std::wstring GetWString(PacketHeader header);

			static std::wstring GetWString(std::shared_ptr<HeapAllocatedData> heapData, int encodingCode);

		private: static bool SendDataStatic(LPVOID dataPtr);
		public:
			bool Send(DP_MESSAGE_TYPE messageType, void* data, DWORD dataSize);
			bool SendAsync(DP_MESSAGE_TYPE messageType, void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback = {});

		private: static bool SendStringStatic(LPVOID dataPtr);
		public:
			bool Send(std::string message);
			bool Send(DP_MESSAGE_TYPE messageType, std::string message);
			bool SendAsync(DP_MESSAGE_TYPE messageType, std::string message, std::function<void(IDPipe* pipe)> callback = {});

		private: static bool SendWStringStatic(LPVOID dataPtr);
		public:
			IDPipe* GetPipe() const;
			bool Send(std::wstring message);
			bool Send(DP_MESSAGE_TYPE messageType, std::wstring message);
			bool SendAsync(DP_MESSAGE_TYPE messageType, std::wstring message, std::function<void(IDPipe* pipe)> callback = {});

			bool SendMessageString(const std::string message);
			bool SendMessageString(const std::wstring message);
			bool SendMessageStringAsync(const std::string message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendMessageStringAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendMessageData(void* data, DWORD dataSize);
			bool SendMessageDataAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback = {});
			bool SendInfo(const std::string message);
			bool SendInfo(const std::wstring message);
			bool SendInfoAsync(const std::string message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendInfoAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendInfo(void* data, DWORD dataSize);
			bool SendInfoAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback = {});
			bool SendWarning(const std::string message);
			bool SendWarning(const std::wstring message);
			bool SendWarningAsync(const std::string message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendWarningAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendWarning(void* data, DWORD dataSize);
			bool SendWarningAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback = {});
			bool SendError(const std::string message);
			bool SendError(const std::wstring message);
			bool SendErrorAsync(const std::string message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendErrorAsync(const std::wstring message, std::function<void(IDPipe* pipe)> callback = {});
			bool SendError(void* data, DWORD dataSize);
			bool SendErrorAsync(void* data, DWORD dataSize, std::function<void(IDPipe* pipe)> callback = {});

			void SetMessageStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);
			void SetMessageDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);

			void SetInfoStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);
			void SetInfoDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);

			void SetWarningStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);
			void SetWarningDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);

			void SetErrorStringReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);
			void SetErrorDataReceivedHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> function);

			void SetStringEncodingCode(DWORD code);
			void SetWStringEncodingCode(DWORD code);

			void Disconnect();
			void Disconnect(std::string message);
			void Disconnect(std::wstring message);
		};
#pragma endregion IDPipeMessanger

	}
}