#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest8Class : public ClientTest {
public:
	ClientTest8Class(TestRegistrationClient registration) :
		ClientTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	DPWString* _dpwstring = nullptr;

	BoolTrigger disconnectTrigger;
	BoolTrigger messageReceivedTrigger;
	BoolTrigger received10MessagesSyncTrigger;
	BoolTrigger received10MessagesAsyncTrigger;

	static void SendAsyncComplete() {

	}

	void OnMessageReceivedCallback(wstring message) {
		if (message == L"Hello, Client!") {
			WriteClientLine() << "2. Greeting Received" << END_LINE;
			messageReceivedTrigger.SetComplete();
		}
		else if (message == L"MessagesReceivedSync!")
			received10MessagesSyncTrigger.SetComplete();
		else if (message == L"MessagesReceivedAsync!")
			received10MessagesAsyncTrigger.SetComplete();
	}

	static void WriteGreetingToServerFromThreadSync(DPWString* dpstring) {
		wstringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		wstring message(ss.str());
		dpstring->SendAsync(message);
	}

	static void WriteGreetingToServerFromThreadASync(DPWString* dpstring) {
		wstringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		wstring message(ss.str());
		auto callback = []() { SendAsyncComplete(); };
		dpstring->SendAsync(message, callback);
	}

	void WriteFrom10ThreadsSync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send sync) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadSync, _dpwstring);
			th.detach();
		}
	}

	void WriteFrom10ThreadsASync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send async) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadASync, _dpwstring);
			th.detach();
		}
	}

	void ServerDisconnectCallback(wstring disconnectMessage) {
		WriteClientLineW() << L"7. Server Disconecting with message: " << disconnectMessage << END_LINE;
		disconnectTrigger.SetComplete();
	}

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpwstring = new DPWString(_dpipe, true);
		_dpwstring->SetOnMessageReceivedHandler([this](wstring message) { this->OnMessageReceivedCallback(message); });
		_dpwstring->SetOnOtherSideDisconnectHandler([this](wstring disconnectMessage) { ServerDisconnectCallback(disconnectMessage); });
		WriteClientLine() << "1. Connecting to Pipe" << END_LINE;
		_dpwstring->Connect(params.handle, L"I am connect, motherfucker!");
		wait(messageReceivedTrigger);

		WriteClientLine() << "3. Sending message from 10 thread sync" << END_LINE;
		WriteFrom10ThreadsSync();

		WriteClientLine() << "4. Waiting confirmation...";
		wait(received10MessagesSyncTrigger);
		cout << "Complete" << endl;

		WriteClientLine() << "5. Sending message from 10 thread async" << END_LINE;
		WriteFrom10ThreadsASync();
		WriteClientLine() << "6. Waiting confirmation...";
		wait(received10MessagesAsyncTrigger);
		cout << "Complete" << endl;

		wait(disconnectTrigger);
		_dpwstring->Disconnect();

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest8() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test8";
	registration.title = L"DPWString server - sync, client - async";
	registration.description = L"Description: Testing scenario when DPWString server in sync mode and client in async mode";
	registration.createHandler = [registration]() { return new ClientTest8Class(registration); };
	return registration;
}