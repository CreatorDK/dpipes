#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest7Class : public ServerTest {
public:
	ServerTest7Class(TestRegistrationServer registration) :
		ServerTest(registration),
		messageSyncReceivedTrigger(10),
		messageAsyncReceivedTrigger(10)
	{ }

private:
	IDPipe* _dpipe = nullptr;
	DPWString* _dpwstring = nullptr;

	BoolTrigger connectTrigger;
	BoolTrigger disconnectTrigger;
	IntTrigger messageSyncReceivedTrigger;
	IntTrigger messageAsyncReceivedTrigger;

	void ClientConnectCallback(wstring connectMessage) {
		WriteServerLineW() << L"1. Client Connected with message: " << connectMessage << END_LINE;
		WriteServerLineW() << L"2. Sending Greeting to client" << END_LINE;
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

	void ClientDisconnectCallback(wstring disconnectMessage) {
		WriteServerLineW() << L"7. Client Disconecting with message: " << disconnectMessage << END_LINE;
		disconnectTrigger.SetComplete();
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpwstring = new DPWString(_dpipe, false);
		_dpwstring->SetOnClientConnectHandler([this](wstring connectMessage) { ClientConnectCallback(connectMessage); });
		_dpwstring->SetOnMessageReceivedHandler([this](wstring message) { MessageRecevicedCallback(message); });
		_dpwstring->SetOnOtherSideDisconnectHandler([this](wstring disconnectMessage) { ClientDisconnectCallback(disconnectMessage); });

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

		wait(disconnectTrigger);
		_dpwstring->Disconnect();
		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest7() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test7";
	registration.title = L"DPWString server - sync, client - sync";
	registration.description = L"Description: Testing scenario when DPWString server and client handling messages in sync mode";
	registration.createHandler = [registration]() { return new ServerTest7Class(registration); };
	return registration;
}