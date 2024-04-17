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
		class IDPClient;
		class DPClient;
		class DPUser;
#pragma endregion prototypes

		//Struct represents request that create client and sends to the server
#pragma region DPRequest
		struct DPRequest {
			int code = 0;
			int dataType = 0;
			void* data = nullptr;
			DWORD dataSize = 0;
			void* descriptor = 0;
			int descriptorType = 0;
			DWORD descriptorSize = 0;
		};
#pragma endregion DPRequest

		//Struct represents response that client receives from server
#pragma region DPResponse
		struct DPResponse {
		private:
			int _code = 0;
			bool _descriptorAllocated;
			int _descriptorType = 0;
			DWORD _descriptorSize = 0;
			bool _dataAllocated = false;
			int _dataType = 0;
			DWORD _dataSize = 0;
			std::shared_ptr<HeapAllocatedData> _data;
			std::shared_ptr<HeapAllocatedData> _descriptor;
			std::chrono::duration<float> _send_duraction{};
			std::chrono::duration<float> _request_duraction{};
			IDPipe* _dpipe = nullptr;
		public:
			DPResponse(
				int code,
				int descriptorType,
				DWORD descriptorSize,
				bool descriptorAllocated,
				std::shared_ptr<HeapAllocatedData> descriptor,
				int dataType,
				DWORD dataSize,
				std::shared_ptr<HeapAllocatedData> data,
				IDPipe* dpipe);

			~DPResponse();
			DPResponse(const DPResponse&) = delete;
			DPResponse operator=(const DPResponse& obj) = delete;
		public:
			int code() const;
			bool descriptorAllocated() const;
			int descriptorType() const;
			DWORD descriptorSize() const;
			std::shared_ptr<HeapAllocatedData> descriptor();
			void* descriptorPtr() const;
			bool dataAllocated() const;
			int dataType() const;
			DWORD dataSize() const;
			std::shared_ptr<HeapAllocatedData> data();
			void* dataPtr() const;
			void keepDescriptor(bool keepAllocatedMemory);
			bool keepDescriptor();
			void keepData(bool keepAllocatedMemory);
			bool keepData();
			void ReadDescriptor(bool keepAllocatedMemory = false);
			void Read(bool keepAllocatedMemory = false);
			std::chrono::duration<float> sendDuraction() const;
			std::chrono::duration<float> requestDuraction() const;
			std::chrono::duration<float> totalDuraction() const;
			IDPipe* dpipe() const;
			friend class DPClient;
			friend class DPUser;
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
			int descriptorType = 0;
			DWORD descriptorSize = 0;
		};
#pragma endregion DPResponseHeader

		//Struct contains data needs to pass into thread that invoke RequestSend events
#pragma region RequestSend
		struct DPRequestAsyncData {
			IDPClient* client = nullptr;
			DPRequest req;
			std::function<void(std::shared_ptr<DPResponse>)> callback;
			DWORD timeout = 0;
		};
#pragma endregion RequestSend

#pragma region IDPClient

		class IDPClient {
		public:
			virtual std::shared_ptr<DPResponse> SendRequest(const DPRequest& req, unsigned int timeout = 0) = 0;
			virtual std::shared_ptr<DPResponse> SendRequest(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout = 0) = 0;
			virtual std::shared_ptr<DPResponse> SendRequest(int code, void* data, DWORD dataSize, unsigned int timeout = 0) = 0;

			virtual void SendRequestAsync(const DPRequest& req, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0) = 0;
			virtual void SendRequestAsync(int code, int dataType, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0) = 0;
			virtual void SendRequestAsync(int code, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0) = 0;
	};

#pragma endregion IDPClient

		//Class that represents client that sends requests to the server through the dpipes
#pragma region DPClient
		class DPClient : public IDPClient, public IDPipeMessanger {
		protected:
			//flags
			bool _handleResponseAsync = true;

			//buffers
			char* _pBufferRequest = nullptr;
			DWORD _nBufferRequestSize = 0;

			bool _keepDescriptorMemoryAllocated = false;

			bool _keepResponseMemoryAllocated = true;
			DWORD _maxDescriptorMemoryAllocation = DP_MEMORY_DESCRIPTOR_ALLOCATION_LIMIT;
			DWORD _maxResponseMemoryAllocation = DP_MEMORY_DATA_ALLOCATION_LIMIT;

			char* _pBufferResponse = nullptr;
			DWORD _nBufferResponseSize = 0;

			std::list<DPRequestRecord*> _requestList;
			std::shared_mutex _requestListMutex;

			IDPipe* _dpipe;
			std::function<void(IDPipe* pipe, PacketHeader header)> _onServerDisconnect;

		public:
			DPClient(IDPipe* dpipe, bool handleAsync = true);
			~DPClient();
			DPClient(const DPClient&) = delete;
			DPClient operator=(const DPClient& obj) = delete;
		protected:
			void OnServerDisconnecting(IDPipe* pipe, PacketHeader header);
			void OnResponseReceived(PacketHeader& header);
			void OnDataReceived(IDPipe* pipe, PacketHeader header);
			void WaitRequestLoop(DPRequestRecord* requsertRecord, unsigned int timeout = 0);
			void FillResponseHeader(const DPResponseHeader& responseRecord) const;
			static void ReceiveResponseAsync(LPVOID dataPtr);
		public:
			void PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord);

			bool keepDescriptorMemoryAllocated();
			void keepDescriptorMemoryAllocated(bool value);

			bool keepResponseMemoryAllocated();
			void keepResponseMemoryAllocated(bool value);
			DWORD maxDescriptorMemoryAllocation();
			void maxDescriptorMemoryAllocation(DWORD value);
			DWORD maxResponseMemoryAllocation();
			DWORD maxResponseMemoryAllocationMB();
			void maxResponseMemoryAllocation(DWORD value);
			void maxResponseMemoryAllocationMB(DWORD value);

			virtual std::shared_ptr<DPResponse> SendRequest(const DPRequest& req, unsigned int timeout = 0) override;
			virtual std::shared_ptr<DPResponse> SendRequest(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout = 0) override;
			virtual std::shared_ptr<DPResponse> SendRequest(int code, void* data, DWORD dataSize, unsigned int timeout = 0) override;

			virtual void SendRequestAsync(const DPRequest& req, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			virtual void SendRequestAsync(int code, int dataType, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0) override;
			virtual void SendRequestAsync(int code, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0) override;

			void SetOnServerDisconnectCallback(std::function<void(IDPipe* pipe, PacketHeader header)> function);
			void FreeAllocatedMemory();
			void FreeAllocatedMemory(void* data);

			bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize);
			bool Connect(IDPipeHandle* pHandle, std::string message);
			bool Connect(IDPipeHandle* pHandle, std::wstring);
			bool Connect(IDPipeHandle* pHandle);

			bool Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize);
			bool Connect(const std::wstring handleString, std::string message);
			bool Connect(const std::wstring handleString, std::wstring message);
			bool Connect(const std::wstring handleString);


			void Disconnect();
			void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize);
		};
#pragma endregion DPClient

	}
}