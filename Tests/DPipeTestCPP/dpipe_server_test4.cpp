#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest4Class : public ServerTest {
public:
	ServerTest4Class(TestRegistrationServer registration) :
		ServerTest(registration),
		messageSyncReceivedTrigger(10),
		messageAsyncReceivedTrigger(10) { }
private:

	IDPipe* _dpipe = nullptr;
	DPString* _dpstring = nullptr;

	BoolTrigger connectTrigger;
	IntTrigger messageSyncReceivedTrigger;
	IntTrigger messageAsyncReceivedTrigger;

	void ClientConnectCallback(string connectMessage) {
		WriteServerLine() << "1. Client Connected with message: " << connectMessage << END_LINE;
		WriteServerLine() << "2. Sending Greeting to client" << END_LINE;
		_dpstring->Send("Hello, Client!");
		connectTrigger.SetComplete();
	}

	void MessageRecevicedCallback(string message) {
		messageSyncReceivedTrigger.Increase(1);

		if (messageSyncReceivedTrigger.IsComplete())
			messageAsyncReceivedTrigger.Increase(1);

		if (newConsole)
			WriteServerLine() << message << END_LINE;
	}

	void Send10MessagesReceivedSyncConfirmation() {
		_dpstring->Send("MessagesReceivedSync!");
	}

	void Send10MessagesReceivedAsyncConfirmation() {
		_dpstring->Send("MessagesReceivedAsync!");
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpstring = new DPString(_dpipe, false);
		_dpstring->SetOnClientConnectHandler([this](string connectMessage) { ClientConnectCallback(connectMessage); });
		_dpstring->SetOnMessageReceivedHandler([this](string message) { MessageRecevicedCallback(message); });

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
		_dpstring->Disconnect("I am disconnect, motherfucker!");

		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest4() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test4";
	registration.title = L"DPString server - sync, client - async";
	registration.description = L"Description: Testing scenario when DPString server in sync mode and client in async mode";
	registration.createHandler = [registration]() { return new ServerTest4Class(registration); };
	return registration;
}