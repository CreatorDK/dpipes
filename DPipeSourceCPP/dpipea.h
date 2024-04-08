#pragma once
#include "dpipebase.h"

namespace crdk {
	namespace dpipes {

		//Hanlde of anonymous duplex pipe
#pragma region DPipeAnonymousHandle
		class DPipeAnonymousHandle : public IDPipeHandle {
		private:
			DPipeAnonymousHandle(std::wstring readHandleString, std::wstring writeHandleString);
			DPipeAnonymousHandle(std::wstring handleString);
		public:
			DPipeAnonymousHandle(HANDLE hReadHandle, HANDLE hWriteHandle);
			static DPipeAnonymousHandle* Start(std::wstring readHandleString, std::wstring writeHandleString);
			static DPipeAnonymousHandle* Start(std::wstring handleString);
		public:
			DPipeAnonymousHandle(const DPipeAnonymousHandle&) = delete;
			DPipeAnonymousHandle operator=(const DPipeAnonymousHandle& obj) = delete;
			~DPipeAnonymousHandle();
		private:
			HANDLE _hReadHandle;
			HANDLE _hWriteHandle;
		public:
			static const std::wstring DefaultPipeName;
			HANDLE GetReadHandle() const;
			HANDLE GetWriteHandle() const;
			static bool IsAnonymous(const std::wstring handleString);
			static bool IsHexHandle(const std::wstring handleString);
			virtual std::wstring AsString() const override;
			virtual DPIPE_TYPE GetType() const override;
		};
#pragma endregion DPipeAnonymousHandle

		//Kind of Duplex pipe (idpipe implementation). Works over anonymous pipes
#pragma region DPipeAnonymous
		class DPipeAnonymous : public IDPipe {
		public:
			DPipeAnonymous(DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT,
				DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);
			DPipeAnonymous(const std::wstring& sName,
				DWORD nInBufferSize = PIPE_BUFFERLENGTH_DEFAULT,
				DWORD nOutBufferSize = PIPE_BUFFERLENGTH_DEFAULT);

			DPipeAnonymous(const DPipeAnonymous&) = delete;
			DPipeAnonymous operator=(const DPipeAnonymous& obj) = delete;
			~DPipeAnonymous();
		private:
			HANDLE _hReadPipeClient = nullptr;
			HANDLE _hWritePipeClient = nullptr;

			static void ReadLoop(LPVOID dataPtr);
			virtual void OnPipeClientConnect() override;
		public:
			virtual bool IsAlive() const override;
			virtual DPIPE_MODE Mode() const override;
			virtual DPIPE_TYPE Type() const override;
			virtual bool Start() override;
			virtual std::shared_ptr<IDPipeHandle> GetHandle() override;
			virtual std::wstring GetHandleString() override;

			virtual bool Connect(IDPipeHandle* pHandle) override;
			virtual bool Connect(IDPipeHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize) override;
			virtual bool Connect(DPipeAnonymousHandle* pHandle);
			virtual bool Connect(DPipeAnonymousHandle* pHandle, LPCVOID pConnectData, DWORD nConnectDataSize);
			virtual bool Connect(std::wstring handleString) override;
			virtual bool Connect(std::wstring handleString, LPCVOID pConnectData, DWORD nConnectDataSize) override;

			virtual void DisconnectPipe() override;

			virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) override;
			virtual bool Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool Write(unsigned int serviceCode, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
			virtual bool WritePacketHeader(PacketHeader header) override;
			virtual bool WriteRaw(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfByteWritten) override;
		};
#pragma endregion DPipeAnonymous

	}
}