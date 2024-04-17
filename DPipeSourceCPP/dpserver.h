#pragma once
#include <vector>
#include <boost/numeric/conversion/cast.hpp>
#include "dpmessangerbase.h"

#define DPWSTRING_BUFFER_SIZE 4096

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class IDPServer;
		class DPServer;
		struct DPReceivedRequest;

#pragma endregion prototypes

		//Represents response that request`s handler sends back to client
#pragma region DPSendingResponse
		struct DPSendingResponse {
		private:
			GUID _guid;
			DPSendingResponse(GUID guid);
		public:
			GUID guid() const;
			int code = 0;
			int dataType = 0;
			void* data = nullptr;
			DWORD dataSize = 0;
			int descriptorType = 0;
			DWORD descriptorSize = 0;
			friend struct DPReceivedRequest;
		};
#pragma endregion DPSendingResponse

		//Represents request received by server from client
#pragma region DPReceivedRequest
		struct DPReceivedRequest {
		public:
			DPReceivedRequest(
				GUID guid, 
				int code,
				int descriptorType,
				DWORD descriptorSize,
				bool descriptorAllocated,
				std::shared_ptr<HeapAllocatedData> descriptor,
				int dataType,
				DWORD dataSize,
				std::shared_ptr<HeapAllocatedData> data,
				IDPServer* server);
			~DPReceivedRequest();
			//DPReceivedRequest(const DPReceivedRequest&) = delete;
			DPReceivedRequest operator=(const DPReceivedRequest& obj) = delete;
		private:
			GUID _guid;
			int _code = 0;
			bool _descriptorAllocated;
			int _descriptorType = 0;
			DWORD _descriptorSize = 0;
			std::shared_ptr<HeapAllocatedData> _descriptor;
			bool _dataAllocated = false;
			int _dataType = 0;
			DWORD _dataSize = 0;
			std::shared_ptr<HeapAllocatedData> _data;
			IDPServer* _server;
		public:
			GUID guid() const;
			int code() const;
			int descriptorType() const;
			bool descriptorAllocated() const;
			DWORD descriptorSize() const;
			void* descriptorPtr() const;
			std::shared_ptr<HeapAllocatedData> descriptor();
			int dataType() const;
			bool dataAllocated() const;
			DWORD dataSize() const;
			void* dataPtr() const;
			std::shared_ptr<HeapAllocatedData> data();
			void keepDescriptor(bool keepAllocatedMemory);
			bool keepDescriptor();
			void keepData(bool keepAllocatedMemory);
			bool keepData();
			void ReadDescriptor(bool keepAllocatedMemory = false);
			void Read(bool keepAllocatedMemory = false);
			DPSendingResponse createRespose();
			IDPServer* server() const;
		};
#pragma endregion DPReceivedRequest

		//Represents request header that received from client through the dpipe stream
#pragma region DPRequestHeader
		struct DPRequestHeader {
			GUID guid = {};
			int code = 0;
			int dataType = 0;
			int descriptorType = 0;
			DWORD descriptorSize = 0;
		};

#pragma endregion DPRequestHeader

		//Represents handler  (function) that execute on request with specific number
#pragma region DPHandler
		struct DPHandler {
			int code = 0;
			std::function<void(DPReceivedRequest&)> function;
		};
#pragma endregion DPHandler

		//Struct contains data needs to pass into thread that invoke on RequestReceived events
#pragma region DPRequestReceivedAsyncData
		struct DPRequestReceivedAsyncData {
			IDPServer* server = nullptr;
			DPRequestHeader reqRecord;
			DPReceivedRequest request;
			IDPipe* pipe = nullptr;
			DPRequestReceivedAsyncData(IDPServer* serverPtr, DPRequestHeader reqRec, DPReceivedRequest clientRequest, IDPipe* pipePtr);
		};
#pragma endregion DPRequestReceivedAsyncData

#pragma region IDPServer
		class IDPServer {
		public:
			virtual IDPipe* Pipe() = 0;
			virtual void OnRequestReceived(IDPipe* pipe, DPRequestHeader& reqRecord, DPReceivedRequest req) = 0;
			virtual void SendResponse(const DPReceivedRequest& req, const DPSendingResponse& resp) = 0;
		};
#pragma endregion IDPServer

		//Class that represents server that receive request from client through the dpipes
#pragma region DPServer
		class DPServer : public IDPServer, public IDPipeMessanger {
		private:
			//flags
			bool _handleRequestAsync = true;

			//buffers
			char* _pBufferRequest = nullptr;
			DWORD _nBufferRequestSize = 0;
			char* _pBufferResponse = nullptr;
			DWORD _nBufferResponseSize = 0;

			bool _keepDescriptorMemoryAllocated = false;

			bool _keepRequestMemoryAllocated = true;
			DWORD _maxDescriptorMemoryAllocation = DP_MEMORY_DESCRIPTOR_ALLOCATION_LIMIT;
			DWORD _maxRequestMemoryAllocation = DP_MEMORY_DATA_ALLOCATION_LIMIT;

			std::vector<DPHandler> _handlers;

			std::shared_mutex _handlerMutex;
			//callbacks
			std::function<void(IDPipe* pipe, PacketHeader header)> _onClientConnect;
			std::function<void(IDPipe* pipe, PacketHeader header)> _onClientDisconnect;
		public:
			//constructors
			DPServer(IDPipe* dpipe, bool handleAsync = true);
			~DPServer();
			DPServer(const DPServer&) = delete;
			DPServer operator=(const DPServer& obj) = delete;
		private:
			//private methods
			void OnClientConnect(IDPipe* pipe, PacketHeader header);
			void OnClientDisconnect(IDPipe* pipe, PacketHeader header);
			void OnDataReceived(IDPipe* pipe, PacketHeader header);
			virtual void OnRequestReceived(IDPipe* pipe, DPRequestHeader& reqRecord, DPReceivedRequest req) override;
			static void OnRequestReceivedAsync(LPVOID dataPtr);
			void FillRequestRecord(const DPRequestHeader& reqRecord) const;
			void PrepareResponce(const DPSendingResponse& response, const GUID& guid);
		public:
			//public methods
			virtual IDPipe* Pipe() override;

			bool keepDescriptorMemoryAllocated();
			void keepDescriptorMemoryAllocated(bool value);

			bool keepRequestMemoryAllocated();
			void keepRequestMemoryAllocated(bool value);
			DWORD maxDescriptorMemoryAllocation();
			void maxDescriptorMemoryAllocation(DWORD value);
			DWORD maxRequestMemoryAllocation();
			DWORD maxRequestMemoryAllocationMB();
			void maxRequestMemoryAllocation(DWORD value);
			void maxRequestMemoryAllocationMB(DWORD value);

			void SetHandler(int code, std::function<void(DPReceivedRequest&)> function);
			void RemoveHandler(int code);

			virtual void SendResponse(const DPReceivedRequest& req, const DPSendingResponse& resp) override;
			void SetOnClientConnectCallback(std::function<void(IDPipe* pipe, PacketHeader header)> function);
			void SetOnClientDisconnectCallback(std::function<void(IDPipe* pipe, PacketHeader header)> function);

			void FreeAllocatedMemory(void* data);
			void FreeAllocatedMemory();

			void Disconnect();
			void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize);
			bool Start();
			std::shared_ptr<IDPipeHandle> GetHandle();
		};
#pragma endregion DPServer

	}
}