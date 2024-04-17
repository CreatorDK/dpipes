#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTestFileTransferClass : public ServerTest {
public:
	ServerTestFileTransferClass(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:

	IDPipe* _dpipeServer = nullptr;
	IDPipe* _dpipeClient = nullptr;

	wstring fileName;
	BoolTrigger testEndTrigger;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader packet) {
		WriteServerLine() << "1. Client Connected" << END_LINE;
	}

	void ClientDisconnectCallback(IDPipe* pipe, PacketHeader packet) {
		WriteServerLine() << "2. Client Disconecting" << END_LINE;
	}

	void PacketHeaderRecevicedCallback(IDPipe* pipe, PacketHeader header) {

		DWORD dataSize = header.DataSize();
		
		wstring newFileName = fileName + L"_";

		std::fstream file(newFileName, std::ios::out | std::ios::binary);

		char* buffer = (char*)malloc(10240);

		auto cycles = header.DataSize() / 10240;
		auto rest = header.DataSize() % 10240;

		for (DWORD i = 0; i < cycles; i++) {
			pipe->Read(buffer, 10240);
			file.write(buffer, 10240);
		}

		pipe->Read(buffer, rest);
		file.write(buffer, rest);

		file.close();

		testEndTrigger.SetComplete();
	}

	void SendDataFile(wstring fileName) {

		std::ifstream file(fileName, std::ios::binary | std::ios::ate);
		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::cout << "File size is " << fileSize << endl;

		char* buffer = (char*)malloc(fileSize);

		auto size = boost::numeric_cast<DWORD>(fileSize);

		if (file.read(buffer, size))
		{
			std::cout << "File is read" << endl;
			DWORD nBytesWritten;
			_dpipeClient->Write(buffer, size, &nBytesWritten);
		}
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipeServer = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpipeServer->SetClientConnectCallback([this](IDPipe* pipe, PacketHeader packet) { ClientConnectCallback(pipe, packet); });
		_dpipeServer->SetPacketHeaderRecevicedCallback([this](IDPipe* pipe, PacketHeader packet) { PacketHeaderRecevicedCallback(pipe, packet); });
		_dpipeServer->SetOtherSideDisconnectCallback([this](IDPipe* pipe, PacketHeader packet) { ClientDisconnectCallback(pipe, packet); });

		_dpipeServer->Start();
		auto handleString = _dpipeServer->GetHandleString();

		_dpipeClient = DPipeBuilder::Create(handleString);
		_dpipeClient->Connect(handleString);

		for(auto flag : params.flags)
		{
			if (flag.first == L"/file" && flag.second.length() > 0)
				fileName = flag.second;
		}

		if (fileName.length() <= 0) {
			WriteServerLineW() << L"Unable to get file name" << END_LINE;
			return;
		}

		SendDataFile(fileName);
		Sleep(500);
		wait(testEndTrigger);
		cout << endl;
	}
};

TestRegistrationServer ServerTestFileTransfer() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"FileTransferTest";
	registration.title = L"File Transfer Test";
	registration.description = L"Description: Testing transfering file throw the dpipe";
	registration.createHandler = [registration]() { return new ServerTestFileTransferClass(registration); };
	return registration;
}