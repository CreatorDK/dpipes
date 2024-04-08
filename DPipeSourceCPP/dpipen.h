#pragma once
#include "dpipebase.h"

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPipeNamed;
#pragma endregion prototypes

		//Hanlde of named duplex pipe
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
			virtual DPIPE_TYPE GetType() const override;

			static std::wstring GetNamedPipeNamePart(std::wstring handleString);
			static std::wstring GetServerNamePart(std::wstring handleString);

			friend class DPipeNamed;
		};
#pragma endregion DuplexPipeNamedHandle

		//Kind of Duplex pipe (idpipe implementation). Works over named pipes
#pragma region DuplexPipeNamed
		class DPipeNamed : public IDPipe {
		public:
			DPipeNamed(DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT,
				DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);
		private:
			DPipeNamed(const std::wstring& name,
				DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT,
				DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);
		public:
			static DPipeNamed* Create(const std::wstring& name,
				DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT,
				DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);

			DPipeNamed(const DPipeNamed&) = delete;
			DPipeNamed operator=(const DPipeNamed& obj) = delete;
			~DPipeNamed();
		private:
			bool _isAlive = false;
			bool _bPipeWriteConnected = false;
			bool _useRemote = false;

			static void ReadLoop(LPVOID lpVoid);
			bool WriteInner(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten);
			virtual void OnPipeClientConnect() override;
		public:
			virtual bool IsAlive() const override;
			virtual bool Start() override;
			virtual DPIPE_MODE Mode() const override;
			virtual DPIPE_TYPE Type() const override;
			void SetUseRemote(bool value);

			virtual std::shared_ptr<IDPipeHandle> GetHandle() override;
			virtual std::wstring GetHandleString() override;

			virtual bool Start(LPSECURITY_ATTRIBUTES lpSecurityAttributes);

			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(IDPipeHandle* pHandle, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(IDPipeHandle* pHandle) override;
			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) override;
			virtual bool Connect(std::wstring handleString, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(std::wstring handleString) override;
			virtual bool Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
			virtual bool Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) override;

			virtual void DisconnectPipe() override;

			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) override;

			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool Write(unsigned int serviceCoe, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool WritePacketHeader(PacketHeader header) override;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
		};
#pragma endregion DuplexPipeNamed

	}
}