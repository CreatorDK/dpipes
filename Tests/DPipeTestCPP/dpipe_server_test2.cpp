#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest2Class : public ServerTest {
public:
	ServerTest2Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	BoolTrigger connectTrigger;
	BoolTrigger dataReceivedTrigger;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader packet) {
		DWORD nBytesWritten;
		WriteServerLine() << "1. Client Connected" << END_LINE;
		WriteServerLine() << "2. Writing Greeting to Client" << END_LINE;
		string message("Hello, Client!");
		DWORD len = boost::numeric_cast<DWORD>(message.length());
		_dpipe->Write(message.data(), len, &nBytesWritten);
		connectTrigger.SetComplete();
	}

	void PacketHeaderRecevicedCallback(IDPipe* pipe, PacketHeader header) {
		DWORD nBytesToRead = header.DataSize();
		DWORD nBytesRead;
		char* buf = (char*)malloc(nBytesToRead);
		bool bReturn = _dpipe->Read(buf, nBytesToRead, &nBytesRead, NULL);

		if (buf != nullptr) {
			string message(buf, nBytesRead);

			if (message == "Hello, Server!") {
				WriteServerLine() << "3. Greeting Received" << END_LINE;
			}
		}

		free(buf);
		dataReceivedTrigger.SetComplete();
	}

public:
	void Execute(start_params_server& params) override {

		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");
		_dpipe->SetClientConnectCallback([this](IDPipe* pipe, PacketHeader header) {ClientConnectCallback(pipe, header); });
		_dpipe->SetPacketHeaderRecevicedCallback([this](IDPipe* pipe, PacketHeader header) {PacketHeaderRecevicedCallback(pipe, header); });
		_dpipe->Start();
		auto handle = _dpipe->GetHandle();

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		RunClientProccess(&si, &pi, params, handle.get()->AsString(), _name);
		wait(connectTrigger);
		wait(dataReceivedTrigger);
		WriteServerLine() << "4. Disconnecting" << END_LINE;
		_dpipe->Disconnect();
		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest2() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test2";
	registration.title = L"Innitiator disconnection";
	registration.description = L"Description: Testing scenario when client connecting to existing pipe and inniciator disconnection from pipe from before client";
	registration.createHandler = [registration]() { return new ServerTest2Class(registration); };
	return registration;
}