#pragma once
#include <list>
#include <shared_mutex>
#include <chrono>
#include <boost/numeric/conversion/cast.hpp>
#include "dpclient.h"
#include <fstream>

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPUser;
#pragma endregion prototypes

		//Class that represents client that sends requests to the server through the dpipes
#pragma region DPUser
		class DPUser : public IDPipeMessanger, public IDPClient, public IDPServer {
		private:
			//flags
			bool _handleAsync = true;
			//buffers
			char* _pBufferRequest = nullptr;
			DWORD _nBufferRequestSize = 0;

			char* _pBufferResponse = nullptr;
			DWORD _nBufferResponseSize = 0;

			bool _keepDescriptorMemoryAllocated = false;
			bool _keepRequestMemoryAllocated = false;
			bool _keepResponseMemoryAllocated = false;

			DWORD _maxDescriptorMemoryAllocation = DP_MEMORY_DESCRIPTOR_ALLOCATION_LIMIT;
			DWORD _maxRequestMemoryAllocation = DP_MEMORY_DATA_ALLOCATION_LIMIT;
			DWORD _maxResponseMemoryAllocation = DP_MEMORY_DATA_ALLOCATION_LIMIT;

			std::vector<DPHandler> _handlers;
			std::shared_mutex _handlerMutex;

			std::list<DPRequestRecord*> _requestList;
			std::shared_mutex _requestListMutex;

			std::function<void(IDPipe* pipe, PacketHeader header)> _onOtherSideConnecting;
			std::function<void(IDPipe* pipe, PacketHeader header)> _onOtherSideDisconnecting;

		public:
			DPUser(IDPipe* dpipe, bool handleAsync = true);
			~DPUser();
			DPUser(const DPUser&) = delete;
			DPUser operator=(const DPUser& obj) = delete;
		private:
			void OnOtherSideConnecting(IDPipe* pipe, PacketHeader header);
			void OnOtherSideDisconnecting(IDPipe* pipe, PacketHeader header);
			void OnDataReceived(IDPipe* pipe, PacketHeader header);
			void OnResponseReceived(PacketHeader& header);
			virtual void OnRequestReceived(IDPipe* pipe, DPRequestHeader& reqRecord, DPReceivedRequest req) override;
			static void OnRequestReceivedAsync(LPVOID dataPtr);
			void WaitRequestLoop(DPRequestRecord* requsertRecord, unsigned int timeout = 0);
			void FillRequestRecord(const DPRequestHeader& reqRecord) const;
			void PrepareResponce(const DPSendingResponse& response, const GUID& guid);
			void FillResponseHeader(const DPResponseHeader& responseRecord) const;
			static void ReceiveResponseAsync(LPVOID dataPtr);
		public:
			virtual IDPipe* Pipe() override;
			void PrepareRequestRecord(const DPRequest& request, const DPRequestRecord* requestRecord);
			std::shared_mutex* GetWriteMutex();

			char* RequestBuffer();
			char* ResponseBuffer();

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

			bool keepRequestMemoryAllocated();
			void keepRequestMemoryAllocated(bool value);

			DWORD maxRequestMemoryAllocation();
			DWORD maxRequestMemoryAllocationMB();
			void maxRequestMemoryAllocation(DWORD value);
			void maxRequestMemoryAllocationMB(DWORD value);

			void SetHandler(int code, std::function<void(DPReceivedRequest&)> function);
			void RemoveHandler(int code);
			virtual void SendResponse(const DPReceivedRequest& req, const DPSendingResponse& resp) override;

			std::shared_ptr<DPResponse> SendRequest(const DPRequest& req, unsigned int timeout = 0);
			std::shared_ptr<DPResponse> SendRequest(int code, int dataType, void* data, DWORD dataSize, unsigned int timeout = 0);
			std::shared_ptr<DPResponse> SendRequest(int code, void* data, DWORD dataSize, unsigned int timeout = 0);

			void SendRequestAsync(const DPRequest& req, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			void SendRequestAsync(int code, int dataType, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			void SendRequestAsync(int code, void* data, DWORD dataSize, std::function<void(std::shared_ptr<DPResponse>)> receiveResponseCallback, unsigned int timeout = 0);
			
			void SetOnOtherSideConnectCallback(std::function<void(IDPipe* pipe, PacketHeader header)> function);
			void SetOnOtherSideDisconnectCallback(std::function<void(IDPipe* pipe, PacketHeader header)> function);
			void FreeAllocatedMemory();
			void FreeAllocatedMemory(void* data);

			void FreeAllocatedMemory(unsigned long long dataAddress); //experimental

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

			static std::function<void(void* address, DWORD size)> _onMemoryAllocatedCallback;
			static std::function<void(void* address, DWORD size)> _onMemoryDeallocatedCallback;
		public:
			static void SetMemoryAllocatedCallback(std::function<void(void* address, DWORD size)> function);
			static void SetMemoryDeallocatedCallback(std::function<void(void* address, DWORD size)> function);
			static void ReadAllocatedAddresses(std::list<std::pair<void*, DWORD>>& list);
		};
#pragma endregion DPUser

#pragma region DPFileTransporter

		class DPFileReceiverRequest {
		private:
			std::shared_ptr<HeapAllocatedData> _fileNameData;
			DWORD _encodingCode;
			std::shared_ptr<HeapAllocatedData> _fileData;
			DWORD _fileSize;
			bool _fileDataAllocated;
			IDPServer* _server;
		public:
			DPFileReceiverRequest(
				std::shared_ptr<HeapAllocatedData> fileNameData,
				DWORD encodingCode,
				std::shared_ptr<HeapAllocatedData> fileData,
				DWORD fileSize,
				bool fileDataAllocated,
				IDPServer* server
				);

			std::string GetFileName();
			std::wstring GetFileNameW();
			DWORD GetFileSize();
			void SaveFile(std::ofstream& fs, DWORD bufferSize);
		};

		class DPFileTransporter {
		private:
			DPUser* _dpuser;
			int _code = 0;
			std::function<void(DPFileReceiverRequest&)> _onFileReceiveRequest;

			void OnFileRequestReceived(DPReceivedRequest& request);
			void SendFile(std::ifstream& filestream, void* descriptor, DWORD descriptorSize, DWORD encodingCode, DWORD bufferSize);
		public:
			DPFileTransporter(DPUser* dpuser, int code);
			~DPFileTransporter();
			void SetFileReceivedCallback(std::function<void(DPFileReceiverRequest&)> function);
			void SendFile(std::ifstream& filestream, const std::string& fileName, DWORD bufferSize);
			void SendFile(std::ifstream& filestream, const std::wstring& fileName, DWORD bufferSize);
		};

#pragma endregion DPFileTransporter
	}
}