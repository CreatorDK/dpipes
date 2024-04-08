#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest3Class : public ClientTest {
public:
	ClientTest3Class(TestRegistrationClient registration) :
		ClientTest(registration)
	{ }
private:
	IDPipe* _dpipe = nullptr;
	DPString* _dpstring = nullptr;
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
			if(newConsole)
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

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpstring = new DPString(_dpipe, false);
		_dpstring->SetOnMessageReceivedHandler([this](string message) { this->OnMessageReceivedCallback(message); });
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

		WriteClientLine() << "7. Disconnecting From Pipe" << END_LINE;
		_dpstring->Disconnect("I am disconnect, motherfucker!");

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest3() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test3";
	registration.title = L"DPString server - sync, client - sync";
	registration.description = L"Description: Testing scenario when DPString server and client handling messages in sync mode";
	registration.createHandler = [registration]() { return new ClientTest3Class(registration); };
	return registration;
}