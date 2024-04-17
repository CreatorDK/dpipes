#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest17Class : public ServerTest {
public:
	ServerTest17Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:
	DPipeNamed* _dpipeNamed = nullptr;
	BoolTrigger clientConnectedTrigger;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader packetHeader) {
		WriteServerLine() << "1. Client Connected" << END_LINE;
		clientConnectedTrigger.SetComplete();
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);

		_dpipeNamed = DPipeNamed::Create(L"\\\\.\\pipe\\test-pipe-123");
		_dpipeNamed->SetUseRemote(true);

		_dpipeNamed->SetClientConnectCallback([this](IDPipe* pipe, PacketHeader packet) { ClientConnectCallback(pipe, packet); });

		_dpipeNamed->Start();

		auto handleString = _dpipeNamed->GetHandleString();

		WriteServerLineW() << L"Pipe handle: " << handleString << END_LINE;
		WriteServerLineW() << L"Waiting for the clinet connect" << END_LINE;

		wait(clientConnectedTrigger);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest17() {
	TestRegistrationServer registration;
	registration.enabled = false;
	registration.name = L"Test17";
	registration.title = L"Named Pipe Remote";
	registration.description = L"Description: Testing Named Pipe Connection over Local Network";
	registration.createHandler = [registration]() { return new ServerTest17Class(registration); };
	return registration;
}