#pragma once
#include <shared_mutex>
#include <boost/numeric/conversion/cast.hpp>
#include "dpipe.h"

#define DPWSTRING_BUFFER_SIZE 4096

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPWString;
		static void RunHandleMessageWStringAsync(LPVOID dataPtr);
#pragma endregion prototypes

		//Struct contains callback and wstring to pass (uses to pass into thread)
#pragma region WStringCallbackDataAsync
		struct WStringCallbackDataAsync {
			std::function<void(std::wstring)> callback;
			std::wstring data;
		};
#pragma endregion WStringCallbackDataAsync

		//Struct contains data that will be sent asynchronously and callback function
#pragma region SendWStringDataAsync
		struct SendWStringDataAsync {
			DPWString* dpstring = nullptr;
			std::wstring data;
			std::function<void(void)> callback;
		};
#pragma endregion SendWStringDataAsync

		//Class that implements methods to send and receive wstring over dpipes
#pragma region DPWString
		class DPWString {
		private:
			std::function<void(std::wstring)> _onClientConnected;
			std::function<void(std::wstring)> _onOtherSideDisconnected;
			std::function<void(std::wstring)> _onMessageReceived;

			void OnClientConnect(PacketHeader header);
			void OnOtherSideDisconnect(PacketHeader header);
			void OnMessageReceived(PacketHeader header);
			void ChekBufferSize(DWORD nBytes);
			std::wstring GetStringFromPipe(const PacketHeader& header);
			static void SendAsync(LPVOID dataPtr);
			static void StaticSendWStringAsync(LPVOID dataPtr);

			bool _handleAsync = true;

			wchar_t* _buffer = nullptr;
			DWORD _nBufferSize = 0;

			mutable std::shared_mutex _mutexWritePipe;

			IDPipe* _dpipe;
		public:
			DPWString(IDPipe* dpipe, bool handleAsync = true, DWORD nBufferSize = DPWSTRING_BUFFER_SIZE);
			~DPWString();
			DPWString(const DPWString&) = delete;
			DPWString operator=(const DPWString& obj) = delete;

			void SetOnClientConnectHandler(std::function<void(std::wstring)>);
			void SetOnOtherSideDisconnectHandler(std::function<void(std::wstring)>);
			void SetOnMessageReceivedHandler(std::function<void(std::wstring)>);

			void Send(std::wstring message);
			void SendAsync(std::wstring message, std::function<void(void)> callback = nullptr);
			bool Connect(IDPipeHandle* pHandle, std::wstring connectString = L"");
			bool Connect(std::wstring handle, std::wstring connectString = L"");
			void Disconnect(std::wstring disconnectString = L"");
		};
#pragma endregion DPWString

	}
}