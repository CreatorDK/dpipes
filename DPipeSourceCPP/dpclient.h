#pragma once
#include <list>
#include <shared_mutex>
#include <chrono>
#include <boost/numeric/conversion/cast.hpp>
#include "dpserver.h"

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPClient;
#pragma endregion prototypes

		//Struct represents request that create client and sends to the server
#pragma region DPRequest
		struct DPRequest {
			int code = 0;
			int dataType = 0;
			void* data = nullptr;
			DWORD dataSize = 0;
		};
#pragma endregion DPRequest

		//Struct represents response that client receives from server
#pragma region DPResponse
		struct DPResponse {
		private:
			int _code = 0;
			int _dataType = 0;
			std::shared_ptr<HeapAllocatedData> _data;
			std::chrono::duration<float> _send_duraction{};
			std::chrono::duration<float> _request_duraction{};
		public:
			DPResponse(int code, int dataType, std::shared_ptr<HeapAllocatedData> data);
			~DPResponse();
			DPResponse(const DPResponse&) = delete;
			DPResponse operator=(const DPResponse& obj) = delete;
		public:
			int code() const;
			int dataType() const;
			void* data() const;
			std::shared_ptr<HeapAllocatedData> getData();
			DWORD dataSize() const;
			void keepAllocatedMemory(bool keepAllocatedMemory);
			bool keepAllocatedMemory();
			std::chrono::duration<float> sendDuraction() const;
			std::chrono::duration<float> requestDuraction() const;
			std::chrono::duration<float> totalDuraction() const;
			friend class DPClient;
		};

#pragma endregion DPResponse

		//Struct contains request unique id (guid) and after client receive response holds the response data that returns in place where client sent request
#pragma region DPRequestRecord
		struct DPRequestRecord {
			GUID guid = {};
			bool succes = false;
			std::shared_ptr<DPResponse> resp;
		};
#pragma endregion DPRequestRecord

		//Represents response header that received from server through the dpipe stream
#pragma region DPResponseHeader
		struct DPResponseHeader
		{
			GUID guid = {};
			int code = 0;
			int dataType = 0;
		};
#pragma endregion DPResponseHeader

		//Struct contains data needs to pass into thread that invoke RequestSend events
#pragma region RequestSend
		struct DPRequestAsyncData {
			DPClient* client = nullptr;
			DPRequest req;
			std::function<void(std::shared_ptr<DPResponse>)> callback;
			DWORD timeout = 0;
		};
#pragma endregion RequestSend

		//Class that represents client that sends requests to the server through the dpipes
#pragma region DPClient
		class DPClient : public IDPipeMessanger {
		private:
			//flags
			bool _handleAsync = true;
			//buffers
			char* _pBufferRequest = nullptr;
			DWORD _nBufferRequestSize = 0;

			char* _pBufferResponse = nullptr;
			DWORD _nBufferResponseSize = 0;

			std::list<DPRequestRecord*> _requestList;
			std::shared_mutex _requestListMutex;

			std::function<void(std::shared_ptr<HeapAllocatedData> data)> _onServerDisconnect;

		public:
			DPClient(IDPipe* dpipe, bool handleAsync = true, DWORD nBufferStringSize = DPWSTRING_BUFFER_SIZE);
			~DPClient();
			DPClient(const DPClient&) = delete;
			DPClient operator=(const DPClient& obj) = delete;
		private:
			void OnServerDisconnecting(PacketHeader header);
			void OnResponseReceived(PacketHeader& header);
			void OnDataReceived(PacketHeader header);
			void WaitRequestLoop(DPRequestRecord* requsertRecord, unsigned int timeout = 0);
			void PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord);
			void FillResponseHeader(const DPResponseHeader& responseRecord) const;
			static void ReceiveResponseAsync(LPVOID dataPtr);
		public:
			//public flags
			bool keepResponseAllocatedMemory = false;
			std::shared_ptr<DPResponse> Send(const DPRequest& req, unsigned int timeout = 0);
			std::shared_ptr<DPResponse> Send(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout = 0);
			std::shared_ptr<DPResponse> Send(int code, void* data, DWORD dataSize, unsigned int timeout = 0);

			void SendAsync(const DPRequest& req, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			void SendAsync(int code, int dataType, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			void SendAsync(int code, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			void SetOnServerDisconnectCallback(std::function<void(std::shared_ptr<HeapAllocatedData> data)> function);
			void FreeAllocatedMemory(void* data);

			bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize);
			bool Connect(IDPipeHandle* pHandle);
			bool Connect(const std::wstring handleString);
			bool Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize);
			void Disconnect();
			void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize);
		};
#pragma endregion DPClient

	}
}