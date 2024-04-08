#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest2Class : public ClientTest { 
public:
	ClientTest2Class(TestRegistrationClient registration):
		ClientTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	BoolTrigger messageReceivedTrigger;
	BoolTrigger serverDisconnectTrigger;

	void PacketHeaderReceviced(PacketHeader header) {
		DWORD nBytesToRead = header.DataSize();
		DWORD nBytesRead;
		char* buf = (char*)malloc(nBytesToRead);
		bool bReturn = _dpipe->Read(buf, nBytesToRead, &nBytesRead, NULL);

		if (buf != nullptr) {
			string message(buf, nBytesRead);

			if (message == "Hello, Client!") {
				WriteClientLine() << "2. Greeting Received" << END_LINE;
			}
		}

		free(buf);
		messageReceivedTrigger.SetComplete();
	}

	void OnServerDisconnect(PacketHeader header) {
		WriteClientLine() << "4. Server Disconnecting" << END_LINE;
		serverDisconnectTrigger.SetComplete();
	}

	void WriteGreetingToServer() {
		DWORD nBytesWritten;
		string message("Hello, Server!");
		DWORD len = boost::numeric_cast<DWORD>(message.length());
		_dpipe->Write(message.data(), len, &nBytesWritten);
	}

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpipe->SetPacketHeaderRecevicedCallback([this](PacketHeader header) { PacketHeaderReceviced(header); });
		_dpipe->SetOtherSideDisconnectCallback([this](PacketHeader header) { OnServerDisconnect(header); });
		WriteClientLine() << "1. Connecting to Pipe" << END_LINE;
		_dpipe->Connect(params.handle);
		wait(messageReceivedTrigger);
		WriteClientLine() << "3. Writing Greeting to Server" << END_LINE;
		WriteGreetingToServer();
		wait(serverDisconnectTrigger);

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest2() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test2";
	registration.title = L"Innitiator disconnection";
	registration.description = L"Description: Testing scenario when client connecting to existing pipe and inniciator disconnection from pipe from before client";
	registration.createHandler = [registration]() { return new ClientTest2Class(registration); };
	return registration;
}