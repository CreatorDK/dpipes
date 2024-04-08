#pragma once
#include <shared_mutex>
#include <boost/numeric/conversion/cast.hpp>
#include "dpipe.h"

#define DPSTRING_BUFFER_SIZE 4096

namespace crdk {
	namespace dpipes {

		//Prototypes
#pragma region prototypes
		class DPString;
		static void RunHandleMessageStringAsync(LPVOID dataPtr);
#pragma endregion prototypes

		//Struct contains callback and string to pass (uses to pass into thread)
#pragma region StringCallbackDataAsync
		struct StringCallbackDataAsync {
			std::function<void(std::string)> callback;
			std::string data;
		};
#pragma endregion StringCallbackDataAsync

		//Struct contains data that will be sent asynchronously and callback function
#pragma region SendStringDataAsync
		struct SendStringDataAsync {
			DPString* dpstring = nullptr;
			std::string data;
			std::function<void(void)> callback;
		};
#pragma endregion SendStringDataAsync

		//Class that implements methods to send and receive string over dpipes
#pragma region DPString
		class DPString {
		private:
			std::function<void(std::string)> _onClientConnected;
			std::function<void(std::string)> _onOtherSideDisconnected;
			std::function<void(std::string)> _onMessageReceived;

			void OnClientConnect(PacketHeader header);
			void OnOtherSideDisconnect(PacketHeader header);
			void OnMessageReceived(PacketHeader header);
			void ChekBufferSize(DWORD nBytes);
			std::string GetStringFromPipe(const PacketHeader& header);
			static void StaticSendStringAsync(LPVOID dataPtr);

			bool _handleAsync = true;

			char* _buffer = nullptr;
			DWORD _nBufferSize = 0;

			mutable std::shared_mutex _mutexWritePipe;

			IDPipe* _dpipe;
		public:
			DPString(IDPipe* dpipe, bool handleAsync = true, DWORD nBufferSize = DPSTRING_BUFFER_SIZE);
			~DPString();
			DPString(const DPString&) = delete;
			DPString operator=(const DPString& obj) = delete;

			void SetOnClientConnectHandler(std::function<void(std::string)>);
			void SetOnOtherSideDisconnectHandler(std::function<void(std::string)>);
			void SetOnMessageReceivedHandler(std::function<void(std::string)>);

			void Send(std::string message);
			void SendAsync(std::string message, std::function<void(void)> callback = nullptr);
			bool Connect(IDPipeHandle* pHandle, std::string connectString = "");
			bool Connect(std::wstring handle, std::string connectString = "");
			void Disconnect(std::string disconnectString = "");
		};
#pragma endregion DPString

	}
}