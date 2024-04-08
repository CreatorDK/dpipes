#pragma once
#include <vector>
#include <boost/numeric/conversion/cast.hpp>
#include "dpmessanger.h"

#define DPWSTRING_BUFFER_SIZE 4096

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes

		class DPServer;
		struct DPClientRequest;

#pragma endregion prototypes

		//Represents response that request`s handler sends back to client
#pragma region DPClientResponse
		struct DPClientResponse {
		private:
			GUID _guid;
			DPClientResponse(GUID guid);
		public:
			GUID guid() const;
			int code = 0;
			int dataType = 0;
			void* data = nullptr;
			DWORD dataSize = 0;
			friend struct DPClientRequest;
		};
#pragma endregion DPClientResponse

		//Represents request received by server from client
#pragma region DPClientRequest
		struct DPClientRequest {
		public:
			DPClientRequest(GUID guid, int code, int dataType, std::shared_ptr<HeapAllocatedData> data);
			~DPClientRequest();
			DPClientRequest(const DPClientRequest&) = delete;
			DPClientRequest operator=(const DPClientRequest& obj) = delete;
		private:
			GUID _guid;
			int _code = 0;
			int _dataType;
			std::shared_ptr<HeapAllocatedData> _data;
		public:
			GUID guid() const;
			int code() const;
			int dataType() const;
			void* data() const;
			std::shared_ptr<HeapAllocatedData> getData();
			DWORD dataSize() const;
			void keepAllocatedMemory(bool keepAllocatedMemory);
			bool keepAllocatedMemory();
			DPClientResponse createRespose();
		};
#pragma endregion DPClientRequest

		//Represents request header that received from client through the dpipe stream
#pragma region DPRequestHeaderServer
		struct DPRequestHeader {
			GUID guid = {};
			int code = 0;
			int dataType = 0;
		};

#pragma endregion DPRequestHeaderServer

		//Represents handler  (function) that execute on request with specific number
#pragma region DPHandler
		struct DPHandler {
			int code = 0;
			std::function<void(DPClientRequest&)> function;
		};
#pragma endregion DPHandler

		//Struct contains data needs to pass into thread that invoke on RequestReceived events
#pragma region DPRequestReceivedAsyncData
		struct DPRequestReceivedAsyncData {
			DPServer* serverPtr = nullptr;
			DPRequestHeader reqRecord;
			std::shared_ptr<HeapAllocatedData> data;
			DPRequestReceivedAsyncData() { }
		};
#pragma endregion DPRequestReceivedAsyncData

		//Class that represents server that receive request from client through the dpipes
#pragma region DPServer
		class DPServer : public IDPipeMessanger {
		private:
			//flags
			bool _handleAsync = true;
			//buffers
			char* _pBufferRequest = nullptr;
			DWORD _nBufferRequestSize = 0;
			char* _pBufferResponse = nullptr;
			DWORD _nBufferResponseSize = 0;

			std::vector<DPHandler> _handlers;

			std::shared_mutex _handlerMutex;
			//callbacks
			std::function<void(std::shared_ptr<HeapAllocatedData> heapData)> _onClientConnect;
			std::function<void(std::shared_ptr<HeapAllocatedData> heapData)> _onClientDisconnect;
		public:
			//constructors
			DPServer(IDPipe* dpipe, bool handleAsync = true, DWORD nBufferStringSize = DPWSTRING_BUFFER_SIZE);
			~DPServer();
			DPServer(const DPServer&) = delete;
			DPServer operator=(const DPServer& obj) = delete;
		private:
			//private methods
			void OnClientConnect(PacketHeader header);
			void OnClientDisconnect(PacketHeader header);
			void OnRequestReceived(DPRequestHeader& reqRecord, std::shared_ptr<HeapAllocatedData> data);
			static void OnRequestReceivedAsync(LPVOID dataPtr);
			void OnDataReceived(PacketHeader header);
			void FillRequestRecord(const DPRequestHeader& reqRecord) const;
			void PrepareResponce(const DPClientResponse& response, const GUID& guid);
		public:
			//public flags
			bool keepRequestAllocatedMemory = false;
			//public methods
			void SetHandler(int code, std::function<void(DPClientRequest&)> function);
			void SendResponse(const DPClientRequest& req, const DPClientResponse& resp);
			void SetOnClientConnectCallback(std::function<void(std::shared_ptr<HeapAllocatedData> heapData)> function);
			void SetOnClientDisconnectCallback(std::function<void(std::shared_ptr<HeapAllocatedData> heapData)> function);
			void FreeAllocatedMemory(void* data);
			void Disconnect();
			void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize);
			bool Start();
			std::shared_ptr<IDPipeHandle> GetHandle();
		};
#pragma endregion DPServer

	}
}