#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;

#define SPEEDTEST_DEFUALT_DATA_BLOC_SIZE 1024*1024*1024;
#define SPEEDTEST_DEFUALT_DATA_BLOC_COUNT 1024;

class ClientTest16Class : public ClientTest {
public:
	ClientTest16Class(TestRegistrationClient registration) :
		ClientTest(registration) {
	}
private:
	DWORD _bufferSize = 128896;
	void* _buffer = nullptr;
	IDPipe* _dpipe = nullptr;

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_buffer = malloc(_bufferSize);

		WriteClientLine() << "1. Connecting and send data" << END_LINE;
		_dpipe->Connect(params.handle, _buffer, _bufferSize);

		Sleep(100);

		WriteClientLine() << "2. Send data" << END_LINE;
		DWORD nBytesWritten;
		_dpipe->Write(_buffer, _bufferSize, &nBytesWritten);

		Sleep(100);

		WriteClientLine() << "3. Disconecting and send data" << END_LINE;
		_dpipe->Disconnect(_buffer, _bufferSize);

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest16() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test16";
	registration.title = L"Test Skip Method";
	registration.description = L"Description: Testing Skip method when run manualy and in case when OnDisconnect function is not set";
	registration.createHandler = [registration]() { return new ClientTest16Class(registration); };
	return registration;
}