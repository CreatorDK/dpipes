#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest1Class : public ServerTest {
public:
	ServerTest1Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:
	IDPipe* _dpipe = nullptr;
	BoolTrigger connectTrigger;
	BoolTrigger disconnectTrigger;
	
	void ClientConnectCallback(PacketHeader packet) {
		DWORD nBytesWritten;
		WriteServerLine() << "1. Client Connected" << END_LINE;
		WriteServerLine() << "2. Writing Greeting to Client" << END_LINE;
		string message("Hello, Client!");
		DWORD len = boost::numeric_cast<DWORD>(message.length());
		_dpipe->Write(message.data(), len, &nBytesWritten);

		connectTrigger.SetComplete();
	}

	void PacketHeaderRecevicedCallback(PacketHeader header) {
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
	}

	void ClientDisconnectCallback(PacketHeader packet) {
		WriteServerLine() << "4. Client Disconecting" << END_LINE;
		disconnectTrigger.SetComplete();
	}

public:
	void Execute(start_params_server& params) override
	{
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType);
		_dpipe->SetClientConnectCallback([this](PacketHeader packet) { ClientConnectCallback(packet); });
		_dpipe->SetPacketHeaderRecevicedCallback([this](PacketHeader packet) { PacketHeaderRecevicedCallback(packet); });
		_dpipe->SetOtherSideDisconnectCallback([this](PacketHeader packet) { ClientDisconnectCallback(packet); });
		_dpipe->Start();

		auto handle = _dpipe->GetHandle();

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		RunClientProccess(&si, &pi, params, handle.get()->AsString(), _name);
		wait(connectTrigger);
		wait(disconnectTrigger);
		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest1() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test1";
	registration.title = L"Client disconnection";
	registration.description = L"Description: Testing scenario when client connecting to existing pipe and first disconnection from it";
	registration.createHandler = [registration]() { return new ServerTest1Class(registration); };
	return registration;
}