#pragma once
#include <string>
#include <map>
#include <type_traits>
#include <exception>
#include <sstream>
#include <thread>
#include <functional>
#include <shared_mutex>
#include "dpipepacket.h"
#include "heapdata.h"

namespace crdk {
	namespace dpipes {

#define DP_BUFFER_SIZE_DEFAULT						0

#define DP_REQUEST									64
#define DP_RESPONSE									128

#define DP_INFO_DATA								10
#define DP_INFO_STRING								11
#define DP_WARNING_DATA								20
#define DP_WARNING_STRING							21
#define DP_ERROR_DATA								30
#define DP_ERROR_STRING								31
#define DP_MESSAGE_DATA								40
#define DP_MESSAGE_STRING							41

#define DP_REQUEST_SIZE								32
#define DP_RESPONSE_SIZE							32

#define DP_DATA_BINARY								0
#define DP_DATA_STRING								1
#define DP_DATA_WSTRING								2
#define DP_DATA_STRING_JSON							3
#define DP_DATA_WSRTING_JSON						4

#define DP_HANDLER_NOT_FOUND						404
#define DP_REQUEST_TIMEOUT							408

#define DP_ENCODING_MASK							2139095040
#define DP_ENCODING_UTF8							0
#define DP_ENCODING_UNICODE							1

#define DP_MEMORY_DESCRIPTOR_ALLOCATION_LIMIT		4096			// 4 Kb
#define DP_MEMORY_DATA_ALLOCATION_LIMIT				33554432		// 32 MB

		//Prototypes
#pragma region prototypes
		class DuplexPipeBuilder;
		class IDPipe;
#pragma endregion prototypes

		//Duplex server type enumeration
#pragma region DP_TYPE
		enum DP_TYPE {
			ANONYMOUS_PIPE = 1,
			NAMED_PIPE = 2
		};
#pragma endregion DP_TYPE

		//Duplex server mode enumeration
#pragma region DP_MODE
		enum DP_MODE {
			UNSTARTED = 0,
			INNITIATOR = 1,
			CLIENT = 2
		};
#pragma endregion DP_MODE

		//Base handle server class. Initiator class creates handle when create the server, second class uses handle to connect
#pragma region IDPipeHandle
		class IDPipeHandle {
		public:
			IDPipeHandle();
			IDPipeHandle(const IDPipeHandle&) = delete;
			IDPipeHandle operator=(const IDPipeHandle& obj) = delete;
			virtual ~IDPipeHandle();
		public:
			virtual DP_TYPE GetType() const = 0;
			virtual std::wstring AsString() const = 0;
		};
#pragma endregion IDPipeHandle

		//Base class for all duplex pipes. Represent duplex chanel work throw different type of transport mechanisms
#pragma region IDPipe
		class IDPipe {
		public:
			IDPipe(const std::wstring& sName, DWORD mtu, DWORD nInBufferSize = DP_BUFFER_SIZE_DEFAULT, DWORD nOutBufferSize = DP_BUFFER_SIZE_DEFAULT);
			IDPipe(const IDPipe&) = delete;
			IDPipe operator=(const IDPipe& obj) = delete;
			virtual ~IDPipe();
		protected:
			DP_TYPE _type = DP_TYPE::ANONYMOUS_PIPE;
			DP_MODE _mode = DP_MODE::UNSTARTED;
			std::wstring _sName;

			PacketBuilder _packetPuilder;

			DWORD _nBytesRead = 0;
			DWORD _nReadThreadId = 0;
			HANDLE _tReadThread = nullptr;
			HANDLE _tServiceThread = nullptr;

			DWORD _nInBufferSize;
			DWORD _nOutBufferSize;
			DWORD _nBytesToRead = 0;
			DWORD _nLastError = 0;

			HANDLE	_hReadPipe = nullptr;		// Handle to a read end of a server.
			HANDLE	_hWritePipe = nullptr;		// Handle to a write end of a server.

			bool _bIsAlive = false;
			bool _bListening = false;
			bool _bOtherSideDisconnecting = false;

			void* _skipBuffer = nullptr;
			DWORD _skipBufferSize = DP_SKIP_BUFFER_SIZE;

			bool _clientEmulating = false;
			DWORD _mtu;

			std::function<void(IDPipe* sender, PacketHeader header)> _onPingReceivedCallBack;
			std::function<void(IDPipe* sender, PacketHeader header)> _onPongReceivedCallBack;
			std::function<void(IDPipe* sender, PacketHeader header)> _onOtherSideConnectCallBack;
			std::function<void(IDPipe* sender, PacketHeader header)> _onOtherSideDisconnectCallBack;
			std::function<void(IDPipe* sender, PacketHeader header)> _onPacketHeaderReceivedCallBack;
			std::function<void(IDPipe* sender, PacketHeader header)> _onConfigurationReceivedCallBack;

			virtual std::wstring GetName() const;
			virtual DWORD BytesToRead() const;
			virtual DWORD GetLastErrorDP() const;

			virtual void ServicePacketReceived(PacketHeader ph);

			virtual void OnClientConnect(PacketHeader ph);
			virtual void OnPipeClientConnect() = 0;
			virtual void OnOtherSideDisconnect(PacketHeader ph);
			virtual void OnOtherSideDisconnectPipe();
			virtual void OnPacketHeaderReceived(PacketHeader ph);
			virtual void OnTerminating(PacketHeader ph);

			virtual void OnPingReceived(PacketHeader ph);
			virtual void OnPongReceived(PacketHeader ph);
			virtual void OnMtuRequest(PacketHeader ph);
			virtual void OnMtuResponse(PacketHeader ph);

			virtual void OnConfigurationReceived(PacketHeader ph);
			virtual void SendConfiguration(LPVOID lpBuffer, DWORD bytes);

			virtual void SendMtuRequest(HANDLE& hWriteHandle, DWORD mtu);
			virtual void SendMtuResponse(HANDLE& hWriteHandle, DWORD mtu);
		public:
			//public flags
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = 0;
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead) = 0;
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) = 0;
			virtual void ReadFrom(IDPipe* source, DWORD nNumberOfBytesToRead, DWORD nBuffurSize);
			virtual std::shared_ptr<HeapAllocatedData> Read(PacketHeader header) = 0;
			virtual bool WritePacketHeader(PacketHeader header) = 0;
			virtual bool WriteRaw(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) = 0;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) = 0;
			virtual bool Write(unsigned int serviceByte, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool Write(unsigned int serviceByte, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) = 0;
			virtual void CopyTo(IDPipe* dest, DWORD nNumberOfBytesToRead, DWORD nBuffurSize);

			virtual DP_MODE Mode() const = 0;
			virtual DP_TYPE Type() const = 0;
			virtual bool IsAlive() const = 0;
			virtual std::shared_ptr<IDPipeHandle> GetHandle() = 0;
			virtual std::wstring GetHandleString() = 0;

			virtual bool SendPing(HANDLE& hWriteHandle);
			virtual bool SendPong(HANDLE& hWriteHandle);

			virtual bool Start() = 0;

			virtual bool Connect(IDPipeHandle* pHandle) = 0;
			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix = 0) = 0;
			virtual bool Connect(const std::wstring handleString) = 0;
			virtual bool Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix = 0) = 0;

			virtual void Disconnect();
			virtual void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix = 0);
			virtual void DisconnectPipe(bool isAlive) = 0;

			virtual void SetPingReceivedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);
			virtual void SetPongReceivedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);
			virtual void SetClientConnectCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);
			virtual void SetOtherSideDisconnectCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);
			virtual void SetPacketHeaderRecevicedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);
			virtual void SetConfigurationRecevicedCallback(std::function<void(IDPipe* sender, PacketHeader ph)> function);

			virtual void Skip(DWORD nBytesToSkip);
			virtual void Skip(PacketHeader header);

			virtual HANDLE ReadHandle();
			virtual HANDLE WriteHandle();

			virtual IDPipe* CreateNewInstance() = 0;

		};
#pragma endregion IDPipe

	}
}