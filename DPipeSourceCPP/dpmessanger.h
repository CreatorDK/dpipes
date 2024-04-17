#pragma once
#include <shared_mutex>
#include <boost/numeric/conversion/cast.hpp>
#include "dpmessangerbase.h"

namespace crdk {
	namespace dpipes {

		//Class that contains common methods both uses in DPServer and DPClient classes for exchange messages
#pragma region DPMessanger 
		class DPMessanger : public IDPipeMessanger {
		public:
			DPMessanger(IDPipe* dpipe, bool handleAsync = true);
			~DPMessanger();
			DPMessanger(const DPMessanger&) = delete;
			DPMessanger operator=(const DPMessanger& obj) = delete;
		private:
			bool _handleAsync = true;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> _onClientConnected;
			std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)> _onOtherSideDisconnected;

			void OnClientConnect(IDPipe* pipe, PacketHeader header);
			void OnOtherSideDisconnect(IDPipe* pipe, PacketHeader header);
			void OnDataReceive(IDPipe* pipe, PacketHeader header);

		public:

			bool Connect(IDPipeHandle* pHandle);
			bool Connect(IDPipeHandle* pHandle, std::string message);
			bool Connect(IDPipeHandle* pHandle, std::wstring message);

			bool Connect(const std::wstring handleString);
			bool Connect(const std::wstring handleString, std::string message);
			bool Connect(const std::wstring handleString, std::wstring message);

			void SetOnClientConnectHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>);
			void SetOnOtherSideDisconnectHandler(std::function<void(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)>);
		};
#pragma endregion DPMessanger

	}
}