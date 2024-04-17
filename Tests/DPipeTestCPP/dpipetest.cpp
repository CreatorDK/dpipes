#include <iostream>
#include <fstream>
#include <map>
#include "common.h"
#include "dpipebase.h"
#include "dpipe_server_tests.h"
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;

HANDLE childProcess;

map<wstring, wstring> GetFlags(int argc, wchar_t* argv[], int start_pos) {
	map<wstring, wstring> result;

	if (argc < start_pos)
		return result;

	for (int i = start_pos; i < argc; i++) {
		wstring arg(argv[i]);
		if (arg[0] = '/') {
			wstring argValue;

			if ((i + 1) < argc) {
				wstring argNext(argv[i + 1]);
				if (argNext[0] != '/') {
					argValue = argNext;
					++i;
				}
			}

			result.insert({ arg, argValue });
		}
	}

	return result;
}

void GetStartParamsServer(int argc, wchar_t* argv[], start_params_server& params) {
	if (argc < 3)
		throw exception("Not enough arguments");

	params.newConsole = false;
	params.path = wstring(argv[0]);
	wstring pipeType = wstring(argv[2]);

	if (pipeType == L"anonymous")
		params.pipeType = DP_TYPE::ANONYMOUS_PIPE;
	else if (pipeType == L"named")
		params.pipeType = DP_TYPE::NAMED_PIPE;
	else if (pipeType == L"all")
		params.pipeType = DP_TYPE::ANONYMOUS_PIPE;
	else {
		throw exception("Unknown pipeType argument");
	}

	if (argc >= 4) {
		wstring arg(argv[3]);
		if (arg[0] != '/') {
			params.test = argv[3];
			params.flags = GetFlags(argc, argv, 4);
		}
		else {
			params.flags = GetFlags(argc, argv, 3);
		}
			
	}
	else {
		params.test = L"all";
	}

	for (const auto& pair : params.flags) {
		if (pair.first == L"/new") {
			params.newConsole = true;
		}
	}
}

void GetStartParamsClient(int argc, wchar_t* argv[], start_params_client& params) {
	if (argc < 3)
		throw exception("Not enough arguments");

	params.newConsole = false;
	params.handle = argv[2];

	if (argc >= 4) {
		wstring arg(argv[3]);
		if (arg[0] != '/') {
			params.test = argv[3];
			params.flags = GetFlags(argc, argv, 4);
		}
		else {
			params.flags = GetFlags(argc, argv, 3);
		}
	}
		
	else {
		params.test = L"all";
	}

	for (const auto& pair : params.flags) {
		if (pair.first == L"/new") {
			params.newConsole = true;
		}
	}
}

int wmain(int argc, wchar_t* argv[]) {

	for (int i = 0; i < argc; i++) {
		wstring arg(argv[i]);

		if (arg == L"/attach") {
			Sleep(3000);
			break;
		}
	}

	if (argc < 3) {
		cout << "Not enough arguments for start tests" << endl;
		system("pause");
		return 1;
	}

	wstring roleString = wstring(argv[1]);
	start_params_server start_params_server;
	start_params_client start_params_client;

	if (roleString == L"server") {

		try {
			GetStartParamsServer(argc, argv, start_params_server);
			if (wstring(argv[2]) == L"all") {
				RunServerTest(start_params_server, true);
			}
			else
				RunServerTest(start_params_server, false);
		}
		catch (exception ex) {
			cout << "Exception occurred while getting startup server parameters: ";
			cout << ex.what() << endl;
			system("pause");
			exit(0);
		}
	}

	else if (roleString == L"client") {
		try {
			GetStartParamsClient(argc, argv, start_params_client);
			RunClientTest(start_params_client);
		}
		catch (exception ex) {
			cout << "Exception occurred while getting startup client parameters: ";
			cout << ex.what() << endl;
			system("pause");
			exit(0);
		}
	}

	else {
		cout << "Unkown role: " << endl;
		system("pause");
	}

	return 0;
}