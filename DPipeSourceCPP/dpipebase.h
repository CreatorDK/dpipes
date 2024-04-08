#pragma once
#include <string>
#include <list>
#include <type_traits>
#include <exception>
#include <windows.h>
#include <sstream>
#include <thread>
#include <functional>
#include "dpipepacket.h"

namespace crdk {
	namespace dpipes {

#define PIPE_BUFFERLENGTH_DEFAULT 19

		//Prototypes
#pragma region prototypes
		class DuplexPipeBuilder;
		class IDPipe;
#pragma endregion prototypes

		//Duplex pipe type enumeration
#pragma region DPIPE_TYPE
		enum DPIPE_TYPE {
			ANONYMOUS_PIPE = 1,
			NAMED_PIPE = 2
		};
#pragma endregion DPIPE_TYPE

		//Duplex pipe mode enumeration
#pragma region DPIPE_MODE
		enum DPIPE_MODE {
			UNSTARTED = 0,
			INNITIATOR = 1,
			CLIENT = 2
		};
#pragma endregion DPIPE_MODE

		//Struct contains buffer adress and size. Depending on flag keepAllocatedMemory at destructor free or not free allocated memory
#pragma region HeapAllocatedData
		struct HeapAllocatedData {
		public:
			HeapAllocatedData(DWORD size, bool keepAllocatedMemory = false);
			~HeapAllocatedData();
			HeapAllocatedData(const HeapAllocatedData&) = delete;
			HeapAllocatedData operator=(const HeapAllocatedData& obj) = delete;
		private:
			void* _data = nullptr;
			DWORD _size = 0;
		public:
			bool keepAllocatedMemory = false;
			void* data() const;
			DWORD size() const;
		};
#pragma endregion HeapAllocatedData

		//Base handle pipe class. Initiator class creates handle when create the pipe, second class uses handle to connect
#pragma region IDPipeHandle
		class IDPipeHandle {
		public:
			IDPipeHandle();
			IDPipeHandle(const IDPipeHandle&) = delete;
			IDPipeHandle operator=(const IDPipeHandle& obj) = delete;
			virtual ~IDPipeHandle();
		public:
			virtual DPIPE_TYPE GetType() const = 0;
			virtual std::wstring AsString() const = 0;
		};
#pragma endregion IDPipeHandle

		//Base class for all duplex pipes. Represent duplex chanel work throw different type of transport mechanisms
#pragma region IDPipe
		class IDPipe {
		public:
			IDPipe(const std::wstring& sName, DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT, DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);
			IDPipe(const IDPipe&) = delete;
			IDPipe operator=(const IDPipe& obj) = delete;
			virtual ~IDPipe();
		protected:
			DPIPE_TYPE _type = DPIPE_TYPE::ANONYMOUS_PIPE;
			DPIPE_MODE _mode = DPIPE_MODE::UNSTARTED;
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

			HANDLE	_hReadPipe = nullptr;		// Handle to a read end of a pipe.
			HANDLE	_hWritePipe = nullptr;		// Handle to a write end of a pipe.

			bool _bIsAlive = false;
			bool _bListening = false;
			bool _bOtherSideDisconnecting = false;

			void* _skipBuffer = nullptr;
			DWORD _skipBufferSize = SKIP_BUFFER_SIZE;

			std::function<void(PacketHeader)> _onClientConnectCallBack;
			std::function<void(PacketHeader)> _onPartnerDisconnectCallBack;
			std::function<void(PacketHeader)> _onPacketHeaderReceivedCallBack;

			virtual std::wstring GetName() const;
			virtual DWORD BytesToRead() const;
			virtual DWORD GetLastErrorDP() const;

			virtual void ServicePacketReceived(PacketHeader ph);
			virtual void OnClientConnect(PacketHeader ph);
			virtual void OnPipeClientConnect() = 0;
			virtual void OnOtherSideDisconnect(PacketHeader ph);
			virtual void OnOtherSideDisconnectPipe();

			virtual void OnPacketHeaderReceived(PacketHeader ph);
			virtual void OnTerminating();
		public:
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) = 0;
			virtual bool WritePacketHeader(PacketHeader header) = 0;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0;
			virtual bool Write(unsigned int serviceByte, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) = 0; // <- You stop here!!!!
			virtual DPIPE_MODE Mode() const = 0;
			virtual DPIPE_TYPE Type() const = 0;
			virtual bool IsAlive() const = 0;
			virtual std::shared_ptr<IDPipeHandle> GetHandle() = 0;
			virtual std::wstring GetHandleString() = 0;

			virtual bool Start() = 0;

			virtual bool Connect(IDPipeHandle* pHandle) = 0;
			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) = 0;
			virtual bool Connect(const std::wstring handleString) = 0;
			virtual bool Connect(const std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) = 0;

			virtual void Disconnect();
			virtual void Disconnect(LPCVOID pConnectData, DWORD nConnectDataSize);
			virtual void DisconnectPipe() = 0;

			virtual void SetClientConnectCallback(std::function<void(PacketHeader ph)> function);
			virtual void SetOtherSideDisconnectCallback(std::function<void(PacketHeader ph)> function);
			virtual void SetPacketHeaderRecevicedCallback(std::function<void(PacketHeader ph)> function);

			virtual void Skip(DWORD nBytesToSkip);
			virtual void Skip(PacketHeader header);

			virtual HANDLE ReadHandle();
			virtual HANDLE WriteHandle();
		};
#pragma endregion IDPipe

	}
}