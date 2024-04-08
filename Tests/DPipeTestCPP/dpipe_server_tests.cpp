#pragma once
#include "dpipe_server_tests.h"
#include "dpipe.h"

using namespace std;
using namespace crdk::dpipes;

std::vector<TestRegistrationServer> _serverRegistrationsClass;

TestRegistrationServer ServerTest1();
TestRegistrationServer ServerTest2();
TestRegistrationServer ServerTest3();
TestRegistrationServer ServerTest4();
TestRegistrationServer ServerTest5();
TestRegistrationServer ServerTest6();
TestRegistrationServer ServerTest7();
TestRegistrationServer ServerTest8();
TestRegistrationServer ServerTest9();
TestRegistrationServer ServerTest10();
TestRegistrationServer ServerTest11();
TestRegistrationServer ServerTest12();
TestRegistrationServer ServerTest13();
TestRegistrationServer ServerTest14();
TestRegistrationServer ServerTest15();
TestRegistrationServer ServerTest16();
TestRegistrationServer ServerTest17();

void RunAllServerTests(start_params_server& params) {
	for (TestRegistrationServer& reg : _serverRegistrationsClass) {
		if (reg.enabled && reg.createHandler) {
			ServerTest* test = reg.createHandler();
			test->ExecuteWrapper(params);
			delete test;
		}
	}
}

void RuServerTest(start_params_server& params, wstring testName) {
	for (TestRegistrationServer& reg : _serverRegistrationsClass) {
		if (reg.name == testName) {
			ServerTest* test = reg.createHandler();
			test->ExecuteWrapper(params);
			delete test;
		}
	}
}

void RunServerTest(start_params_server& params, bool bothPipes) {

	_serverRegistrationsClass.push_back(ServerTest1());
	_serverRegistrationsClass.push_back(ServerTest2());
	_serverRegistrationsClass.push_back(ServerTest3());
	_serverRegistrationsClass.push_back(ServerTest4());
	_serverRegistrationsClass.push_back(ServerTest5());
	_serverRegistrationsClass.push_back(ServerTest6());
	_serverRegistrationsClass.push_back(ServerTest7());
	_serverRegistrationsClass.push_back(ServerTest8());
	_serverRegistrationsClass.push_back(ServerTest9());
	_serverRegistrationsClass.push_back(ServerTest10());
	_serverRegistrationsClass.push_back(ServerTest11());
	_serverRegistrationsClass.push_back(ServerTest12());
	_serverRegistrationsClass.push_back(ServerTest13());
	_serverRegistrationsClass.push_back(ServerTest14());
	_serverRegistrationsClass.push_back(ServerTest15());
	_serverRegistrationsClass.push_back(ServerTest16());
	_serverRegistrationsClass.push_back(ServerTest17());

	if (params.test == L"all") {
		if (bothPipes) {
			params.pipeType = DPIPE_TYPE::ANONYMOUS_PIPE;
			RunAllServerTests(params);
			params.pipeType = DPIPE_TYPE::NAMED_PIPE;
			RunAllServerTests(params);
		}
		else {
			RunAllServerTests(params);
		}
	}
		
	else {
		if (bothPipes) {
			params.pipeType = DPIPE_TYPE::ANONYMOUS_PIPE;
			RuServerTest(params, params.test);
			params.pipeType = DPIPE_TYPE::NAMED_PIPE;
			RuServerTest(params, params.test);
		}
		else {
			RuServerTest(params, params.test);
		}
	}

	system("pause");
}
