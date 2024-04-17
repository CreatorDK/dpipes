#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest6Class : public ServerTest {
public:
	ServerTest6Class(TestRegistrationServer registration) :
		ServerTest(registration),
		messageSyncReceivedTrigger(10),
		messageAsyncReceivedTrigger(10) { }
private:

	IDPipe* _dpipe = nullptr;
	DPMessanger* _dpmessanger = nullptr;

	BoolTrigger connectTrigger;
	IntTrigger messageSyncReceivedTrigger;
	IntTrigger messageAsyncReceivedTrigger;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

		string message = _dpmessanger->GetString(header, data);

		WriteServerLine() << "1. Client Connected with message: " << message << END_LINE;
		WriteServerLine() << "2. Sending Greeting to client" << END_LINE;
		_dpmessanger->Send("Hello, Client!");
		connectTrigger.SetComplete();
	}

	void MessageRecevicedCallback(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

		string message = _dpmessanger->GetString(header, data);

		messageSyncReceivedTrigger.Increase(1);

		if (messageSyncReceivedTrigger.IsComplete())
			messageAsyncReceivedTrigger.Increase(1);

		if (newConsole)
			WriteServerLine() << message << END_LINE;
	}

	void Send10MessagesReceivedSyncConfirmation() {
		_dpmessanger->Send("MessagesReceivedSync!");
	}

	void Send10MessagesReceivedAsyncConfirmation() {
		_dpmessanger->Send("MessagesReceivedAsync!");
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpmessanger = new DPMessanger(_dpipe, true);
		_dpmessanger->SetOnClientConnectHandler([this](IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data)
			{ ClientConnectCallback(pipe, header, data); });
		_dpmessanger->SetMessageStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) 
			{ MessageRecevicedCallback(pipe, header, data); });

		_dpipe->Start();
		auto hadnle = _dpipe->GetHandle();

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		RunClientProccess(&si, &pi, params, hadnle.get()->AsString(), _name);
		wait(connectTrigger);

		WriteServerLine() << "3. Waiting to receive sync sended strings from 10 threads" << END_LINE;
		wait(messageSyncReceivedTrigger);

		WriteServerLine() << "4. Sending 10 messages recieved sync confirmation " << END_LINE;
		Send10MessagesReceivedSyncConfirmation();

		WriteServerLine() << "5. Waiting to receive async sended strings from 10 threads" << END_LINE;
		wait(messageAsyncReceivedTrigger);

		WriteServerLine() << "6. Sending 10 messages recieved async confirmation " << END_LINE;
		Send10MessagesReceivedAsyncConfirmation();
		Sleep(100);

		WriteServerLine() << "7. Disconnecting From Pipe" << END_LINE;
		_dpmessanger->Disconnect("I am disconnect, motherfucker!");

		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest6() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test6";
	registration.title = L"DPMessanger server - async, client - async";
	registration.description = L"Description: Testing scenario when DPMessanger server and client handling messages in async mode";
	registration.createHandler = [registration]() { return new ServerTest6Class(registration); };
	return registration;
}