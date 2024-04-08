#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest6Class : public ClientTest {
public:
	ClientTest6Class(TestRegistrationClient registration) :
		ClientTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	DPString* _dpstring = nullptr;

	BoolTrigger disconnectTrigger;
	BoolTrigger messageReceivedTrigger;
	BoolTrigger received10MessagesSyncTrigger;
	BoolTrigger received10MessagesAsyncTrigger;

	static void SendAsyncComplete() {

	}

	void OnMessageReceivedCallback(string message) {
		if (message == "Hello, Client!") {
			WriteClientLine() << "2. Greeting Received" << END_LINE;
			messageReceivedTrigger.SetComplete();
		}
		else if (message == "MessagesReceivedSync!")
			received10MessagesSyncTrigger.SetComplete();
		else if (message == "MessagesReceivedAsync!")
			received10MessagesAsyncTrigger.SetComplete();
	}

	static void WriteGreetingToServerFromThreadSync(DPString* dpstring) {
		stringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		string message(ss.str());
		dpstring->SendAsync(message);
	}

	static void WriteGreetingToServerFromThreadASync(DPString* dpstring) {
		stringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		string message(ss.str());
		auto callback = []() { SendAsyncComplete(); };
		dpstring->SendAsync(message, callback);
	}

	void WriteFrom10ThreadsSync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send sync) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadSync, _dpstring);
			th.detach();
		}
	}

	void WriteFrom10ThreadsASync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send async) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadASync, _dpstring);
			th.detach();
		}
	}

	void ServerDisconnectCallback(string disconnectMessage) {
		WriteClientLine() << "7. Server Disconecting with message: " << disconnectMessage << END_LINE;
		disconnectTrigger.SetComplete();
	}

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpstring = new DPString(_dpipe, true);
		_dpstring->SetOnMessageReceivedHandler([this](string message) { this->OnMessageReceivedCallback(message); });
		_dpstring->SetOnOtherSideDisconnectHandler([this](string disconnectMessage) { ServerDisconnectCallback(disconnectMessage); });
		WriteClientLine() << "1. Connecting to Pipe" << END_LINE;
		_dpstring->Connect(params.handle, "I am connect, motherfucker!");
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
		_dpstring->Disconnect();

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest6() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test6";
	registration.title = L"DPString server - async, client - async";
	registration.description = L"Description: Testing scenario when DPString server and client handling messages in async mode";
	registration.createHandler = [registration]() { return new ClientTest6Class(registration); };
	return registration;
}