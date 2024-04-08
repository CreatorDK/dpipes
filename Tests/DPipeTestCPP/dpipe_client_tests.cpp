#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;

vector<TestRegistrationClient> _clientRegistrations;

TestRegistrationClient ClientTest1();
TestRegistrationClient ClientTest2();
TestRegistrationClient ClientTest3();
TestRegistrationClient ClientTest4();
TestRegistrationClient ClientTest5();
TestRegistrationClient ClientTest6();
TestRegistrationClient ClientTest7();
TestRegistrationClient ClientTest8();
TestRegistrationClient ClientTest9();
TestRegistrationClient ClientTest10();
TestRegistrationClient ClientTest11();
TestRegistrationClient ClientTest12();
TestRegistrationClient ClientTest13();
TestRegistrationClient ClientTest14();
TestRegistrationClient ClientTest15();
TestRegistrationClient ClientTest16();
TestRegistrationClient ClientTest17();

void RunClientTest(start_params_client& params, wstring testName) {
	for (TestRegistrationClient& reg : _clientRegistrations) {
		if (reg.name == testName) {
			ClientTest* test = reg.createHandler();
			test->ExecuteWrapper(params);
		}
	}
}

void RunClientTest(start_params_client& params) { 

	_clientRegistrations.push_back(ClientTest1());
	_clientRegistrations.push_back(ClientTest2());
	_clientRegistrations.push_back(ClientTest3());
	_clientRegistrations.push_back(ClientTest4());
	_clientRegistrations.push_back(ClientTest5());
	_clientRegistrations.push_back(ClientTest6());
	_clientRegistrations.push_back(ClientTest7());
	_clientRegistrations.push_back(ClientTest8());
	_clientRegistrations.push_back(ClientTest9());
	_clientRegistrations.push_back(ClientTest10());
	_clientRegistrations.push_back(ClientTest11());
	_clientRegistrations.push_back(ClientTest12());
	_clientRegistrations.push_back(ClientTest13());
	_clientRegistrations.push_back(ClientTest14());
	_clientRegistrations.push_back(ClientTest15());
	_clientRegistrations.push_back(ClientTest16());
	_clientRegistrations.push_back(ClientTest17());

	RunClientTest(params, params.test);
}