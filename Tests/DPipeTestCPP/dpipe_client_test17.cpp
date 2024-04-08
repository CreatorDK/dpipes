#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;

class ClientTest17Class : public ClientTest {
public:
	ClientTest17Class(TestRegistrationClient registration) :
		ClientTest(registration) {
	}
private:
	IDPipe* _dpipeNamed = nullptr;
public:
	void Execute(start_params_client& params) override {

		_dpipeNamed = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipeNamed->Type());

		WriteClientLine() << "Waiting wor debugger attached" << END_LINE;

		if (newConsole)
			system("pause");

		WriteClientLine() << "1. Connecting" << END_LINE;

		try {
			_dpipeNamed->Connect(params.handle);
			WriteClientLine() << "2. Connected!" << END_LINE;
		}

		catch (exception ex) {
			WriteClientLine() << "Exception at connection: " << ex.what() << END_LINE;
		}

		system("pause");
	}
};

TestRegistrationClient ClientTest17() {
	TestRegistrationClient registration;
	registration.enabled = false;
	registration.name = L"Test17";
	registration.title = L"Test Skip Method";
	registration.description = L"Description: Testing Skip method when run manualy and in case when OnDisconnect function is not set";
	registration.createHandler = [registration]() { return new ClientTest17Class(registration); };
	return registration;
}