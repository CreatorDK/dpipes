#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest1Class : public ClientTest {
public:
	ClientTest1Class(TestRegistrationClient registration) :
		ClientTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	BoolTrigger messageReceivedTrigger;

	void PacketHeaderReceviced(IDPipe* pipe, PacketHeader header) {
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

		_dpipe->SetPacketHeaderRecevicedCallback([this](IDPipe* pipe, PacketHeader header) { this->PacketHeaderReceviced(pipe, header); });
		WriteClientLine() << "1. Connecting to Pipe" << END_LINE;
		_dpipe->Connect(params.handle);
		wait(messageReceivedTrigger);
		WriteClientLine() << "3. Writing Greeting to Server" << END_LINE;
		WriteGreetingToServer();
		Sleep(100);
		WriteClientLine() << "4. Disconnecting From Pipe" << END_LINE;
		_dpipe->Disconnect();

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest1() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test1";
	registration.title = L"Client disconnection";
	registration.description = L"Description: Testing scenario when client connecting to existing pipe and first disconnection from it";
	registration.createHandler = [registration]() { return new ClientTest1Class(registration); };
	return registration;
}