#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest16Class : public ServerTest {
public:
	ServerTest16Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:
	IDPipe* _dpipe = nullptr;
	BoolTrigger clientDisconnectedTrigger;

	void ClientConnectCallback(PacketHeader packetHeader) {
		WriteServerLine() << "1. Client Connected. Receive " << to_string(packetHeader.DataSize()) << " b. Skipping...";
		_dpipe->Skip(packetHeader);
		cout << "Complete" << endl;
	}

	void ClientDisconnectCallback(PacketHeader packetHeader) {
		WriteServerLine() << "2. Client Disconecting. Receive " << to_string(packetHeader.DataSize()) << " b. Skipping...";
		_dpipe->Skip(packetHeader);
		cout << "Complete" << endl;
		clientDisconnectedTrigger.SetComplete();
	}

	void PacketHeaderRecevicedCallback(PacketHeader packetHeader) {
		WriteServerLine() << "Packet Received " << to_string(packetHeader.DataSize()) << " b. Skipping..." << END_LINE;
		_dpipe->Skip(packetHeader);
		cout << "Complete" << endl;
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);

		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		//_dpipe->SetClientConnectCallback([this](PacketHeader packet) { ClientConnectCallback(packet); });
		_dpipe->SetPacketHeaderRecevicedCallback([this](PacketHeader packet) { PacketHeaderRecevicedCallback(packet); });
		_dpipe->SetOtherSideDisconnectCallback([this](PacketHeader packet) { ClientDisconnectCallback(packet); });

		_dpipe->Start();
		auto hadnle = _dpipe->GetHandle();

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		RunClientProccess(&si, &pi, params, hadnle.get()->AsString(), _name);

		wait(clientDisconnectedTrigger);

		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest16() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test16";
	registration.title = L"Test Skip Method";
	registration.description = L"Description: Testing Skip method when run manualy and in case when OnDisconnect function is not set";
	registration.createHandler = [registration]() { return new ServerTest16Class(registration); };
	return registration;
}