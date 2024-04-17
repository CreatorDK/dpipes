#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <dpipebase.h>

enum ROLE {
	SERVER = 1,
	CLIENT = 2,
	UNKNOWN_ROLE = -1
};

struct start_params_server {
	//start_params_server(const start_params_server&) = delete;
	//start_params_server operator=(const start_params_server& obj) = delete;

	std::wstring path;
	crdk::dpipes::DP_TYPE pipeType = crdk::dpipes::DP_TYPE::ANONYMOUS_PIPE;
	std::wstring test;
	std::map<std::wstring, std::wstring> flags;
	bool newConsole = 0;
};

struct start_params_client {
	//start_params_client(const start_params_client&) = delete;
	//start_params_client operator=(const start_params_client& obj) = delete;

	std::wstring handle;
	std::wstring test;
	std::map<std::wstring, std::wstring> flags;
	bool newConsole = 0;
};

class ServerTest;
class ClientTest;

struct TestRegistrationServer {
	bool enabled = true;
	std::wstring name;
	std::wstring title;
	std::wstring description;

	std::function<ServerTest* ()> createHandler;
};

struct TestRegistrationClient {
	bool enabled = true;
	std::wstring name;
	std::wstring title;
	std::wstring description;

	std::function<ClientTest* ()> createHandler;
};

class ServerTest {
protected:
	bool newConsole = false;
	bool enabled = true;
	std::wstring _name;
	std::wstring _title;
	std::wstring _description;
public:
	ServerTest(TestRegistrationServer registration);
	virtual void ExecuteWrapper(start_params_server& params);
	virtual void Execute(start_params_server& params) = 0;
	void WriteTestName(const crdk::dpipes::DP_TYPE type) const;
};

class ClientTest {
protected:
	bool newConsole = false;
	bool enabled = true;
	std::wstring _name;
	std::wstring _title;
	std::wstring _description;
public:
	ClientTest(TestRegistrationClient registration);
	virtual void ExecuteWrapper(start_params_client& params);
	virtual void Execute(start_params_client& params) = 0;
	void WriteTestName(const crdk::dpipes::DP_TYPE type) const;
};

#define END_LINE '\n'

class WriteServerLine {
public:
	WriteServerLine();
	WriteServerLine& operator<< (const std::string& arg);
	WriteServerLine& operator<< (const char& arg);
};

class WriteServerLineW {
public:
	WriteServerLineW();
	WriteServerLineW& operator<< (const std::wstring& arg);
	WriteServerLineW& operator<< (const wchar_t& arg);
};

class WriteClientLine {
public:
	WriteClientLine();
	WriteClientLine& operator<< (const std::string& arg);
	WriteClientLine& operator<< (const char& arg);
};

class WriteClientLineW {
public:
	WriteClientLineW();
	WriteClientLineW& operator<< (const std::wstring& arg);
	WriteClientLineW& operator<< (const wchar_t& arg);
};


void RunClientProccess(STARTUPINFO* si, PROCESS_INFORMATION* pi, start_params_server& params, std::wstring handleString, std::wstring test);