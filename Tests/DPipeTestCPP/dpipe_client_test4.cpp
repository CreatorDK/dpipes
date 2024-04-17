#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest4Class : public ClientTest {
public:
	ClientTest4Class(TestRegistrationClient registration) :
		ClientTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	DPMessanger* _dpmessanger = nullptr;

	BoolTrigger disconnectTrigger;
	BoolTrigger messageReceivedTrigger;
	BoolTrigger received10MessagesSyncTrigger;
	BoolTrigger received10MessagesAsyncTrigger;

	static void SendAsyncComplete(IDPipe* pipe) {

	}

	void OnMessageReceivedCallback(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

		string message = _dpmessanger->GetString(header, data);

		if (message == "Hello, Client!") {
			WriteClientLine() << "2. Greeting Received" << END_LINE;
			messageReceivedTrigger.SetComplete();
		}
		else if (message == "MessagesReceivedSync!")
			received10MessagesSyncTrigger.SetComplete();
		else if (message == "MessagesReceivedAsync!")
			received10MessagesAsyncTrigger.SetComplete();
	}

	static void WriteGreetingToServerFromThreadSync(DPMessanger* dpstring) {
		stringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		string message(ss.str());
		dpstring->SendMessageString(message);
	}

	static void WriteGreetingToServerFromThreadASync(DPMessanger* dpstring) {
		stringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << "Hello, from " << this_id << "!";
		string message(ss.str());
		auto callback = [dpstring](IDPipe* pipe) { SendAsyncComplete(pipe); };
		dpstring->SendMessageStringAsync(message, callback);
	}

	void WriteFrom10ThreadsSync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send sync) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadSync, _dpmessanger);
			th.detach();
		}
	}

	void WriteFrom10ThreadsASync() const {
		for (int i = 0; i < 10; i++) {
			if (newConsole)
				WriteClientLine() << "Creating thread (Send async) " << to_string(i) << END_LINE;
			std::thread th(WriteGreetingToServerFromThreadASync, _dpmessanger);
			th.detach();
		}
	}

	void ServerDisconnectCallback(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

		string message = _dpmessanger->GetString(header, data);

		WriteClientLine() << "7. Server Disconecting with message: " << message << END_LINE;
		disconnectTrigger.SetComplete();
	}

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpmessanger = new DPMessanger(_dpipe, true);
		_dpmessanger->SetMessageStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) 
			{ this->OnMessageReceivedCallback(pipe, header, data); });
		_dpmessanger->SetOnOtherSideDisconnectHandler([this](IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) 
			{ ServerDisconnectCallback(pipe, header, data); });
		WriteClientLine() << "1. Connecting to Pipe" << END_LINE;
		_dpmessanger->Connect(params.handle, "I am connect, motherfucker!");
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
		_dpmessanger->Disconnect();

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest4() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test4";
	registration.title = L"DPMessanger server - sync, client - async";
	registration.description = L"Description: Testing scenario when DPMessanger server in sync mode and client in async mode";
	registration.createHandler = [registration]() { return new ClientTest4Class(registration); };
	return registration;
}