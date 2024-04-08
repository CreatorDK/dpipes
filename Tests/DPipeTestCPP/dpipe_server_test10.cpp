#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest10Class : public ServerTest {
public:
	ServerTest10Class(TestRegistrationServer registration) :
		ServerTest(registration),
		messageSyncReceivedTrigger(10),
		messageAsyncReceivedTrigger(10) { }
private:

	IDPipe* _dpipe = nullptr;
	DPWString* _dpwstring = nullptr;

	BoolTrigger connectTrigger;
	IntTrigger messageSyncReceivedTrigger;
	IntTrigger messageAsyncReceivedTrigger;

	void ClientConnectCallback(wstring connectMessage) {
		WriteServerLineW() << L"1. Client Connected with message: " << connectMessage << END_LINE;
		WriteServerLine() << "2. Sending Greeting to client" << END_LINE;
		_dpwstring->Send(L"Hello, Client!");
		connectTrigger.SetComplete();
	}

	void MessageRecevicedCallback(wstring message) {
		messageSyncReceivedTrigger.Increase(1);

		if (messageSyncReceivedTrigger.IsComplete())
			messageAsyncReceivedTrigger.Increase(1);

		if (newConsole)
			WriteServerLineW() << message << END_LINE;
	}

	void Send10MessagesReceivedSyncConfirmation() {
		_dpwstring->Send(L"MessagesReceivedSync!");
	}

	void Send10MessagesReceivedAsyncConfirmation() {
		_dpwstring->Send(L"MessagesReceivedAsync!");
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpwstring = new DPWString(_dpipe, true);
		_dpwstring->SetOnClientConnectHandler([this](wstring connectMessage) { ClientConnectCallback(connectMessage); });
		_dpwstring->SetOnMessageReceivedHandler([this](wstring message) { MessageRecevicedCallback(message); });

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
		_dpwstring->Disconnect(L"I am disconnect, motherfucker!");

		Sleep(500);
		std::cout << std::endl;
	}
};
TestRegistrationServer ServerTest10() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test10";
	registration.title = L"DPWString server - async, client - async";
	registration.description = L"Description: Testing scenario when DPWString server and client handling messages in async mode";
	registration.createHandler = [registration]() { return new ServerTest10Class(registration); };
	return registration;
}