#pragma once
#include "dpipebase.h"

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPipeNamed;
#pragma endregion prototypes

		//Hanlde of named duplex server
#pragma region DPipeNamedHandle
		class DPipeNamedHandle : public IDPipeHandle {
		public:
			DPipeNamedHandle(const std::wstring& sServerName, const std::wstring& sPipeName);
			static DPipeNamedHandle* Create(const std::wstring& sPipeNameFull);
			DPipeNamedHandle(const DPipeNamedHandle&) = delete;
			DPipeNamedHandle operator=(const DPipeNamedHandle& obj) = delete;
			~DPipeNamedHandle();
		private:
			std::wstring _sPipeName;
			std::wstring _sServerName;
		public:
			std::wstring GetServerName();
			std::wstring GetPipeName();
		public:			
			static const std::wstring DefaultPipeName;
			static std::wstring GetMachineName();
			static bool IsNamed(const std::wstring& handleString);
			virtual std::wstring AsString() const override;
			virtual DP_TYPE GetType() const override;

			static std::wstring GetNamedPipeNamePart(std::wstring handleString);
			static std::wstring GetServerNamePart(std::wstring handleString);

			friend class DPipeNamed;
		};
#pragma endregion DuplexPipeNamedHandle

		//Kind of Duplex server (idpipe implementation). Works over named pipes
#pragma region DuplexPipeNamed
		class DPipeNamed : public IDPipe {
		public:
			DPipeNamed(DWORD nInBufferSize = DP_BUFFER_SIZE_DEFAULT,
				DWORD nOutBufferSize = DP_BUFFER_SIZE_DEFAULT);
		private:
			DPipeNamed(const std::wstring& name,
				DWORD nInBufferSize = DP_BUFFER_SIZE_DEFAULT,
				DWORD nOutBufferSize = DP_BUFFER_SIZE_DEFAULT);
		public:
			static DPipeNamed* Create(const std::wstring& name,
				DWORD nInBufferSize = DP_BUFFER_SIZE_DEFAULT,
				DWORD nOutBufferSize = DP_BUFFER_SIZE_DEFAULT);

			DPipeNamed(const DPipeNamed&) = delete;
			DPipeNamed operator=(const DPipeNamed& obj) = delete;
			~DPipeNamed();
		private:
			bool _isAlive = false;
			bool _bPipeWriteConnected = false;
			bool _useRemote = false;

			static void ReadLoop(LPVOID lpVoid);
			bool WriteInner(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten);
			virtual void OnPipeClientConnect() override;
		public:
			virtual bool IsAlive() const override;
			virtual bool Start() override;
			virtual DP_MODE Mode() const override;
			virtual DP_TYPE Type() const override;
			void SetUseRemote(bool value);

			virtual std::shared_ptr<IDPipeHandle> GetHandle() override;
			virtual std::wstring GetHandleString() override;

			virtual bool Start(LPSECURITY_ATTRIBUTES lpSecurityAttributes);

			virtual bool ConnectNamed(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD prefix = 0);
			virtual bool ConnectNamed(IDPipeHandle* pHandle, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(IDPipeHandle* pHandle) override;
			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix = 0) override;
			virtual bool ConnectNamed(std::wstring handleString, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(std::wstring handleString) override;
			virtual bool ConnectNamed(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, DWORD prefix = 0) override;

			virtual void DisconnectPipe(bool isAlive) override;

			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) override;
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead) override;
			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) override;
			virtual std::shared_ptr<HeapAllocatedData> Read(PacketHeader header) override;

			virtual bool Write(unsigned int serviceCoe, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) override;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) override;
			virtual bool WritePacketHeader(PacketHeader header) override;
			virtual bool WriteRaw(HANDLE hWriteHandle, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite) override;

			virtual IDPipe* CreateNewInstance() override;
		};
#pragma endregion DuplexPipeNamed

	}
}