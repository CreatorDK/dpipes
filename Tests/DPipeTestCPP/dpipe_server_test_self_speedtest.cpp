#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

#define SPEEDTEST_DEFUALT_DATA_BLOC_SIZE 256*1024*1024;
#define SPEEDTEST_DEFUALT_DATA_BLOC_COUNT 20;

class ServerTestSpeedtestClass : public ServerTest {
public:
	ServerTestSpeedtestClass(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:
	DWORD _blockCount = SPEEDTEST_DEFUALT_DATA_BLOC_COUNT;
	DWORD _bufferSize = SPEEDTEST_DEFUALT_DATA_BLOC_SIZE;

	void* _bufferServer = nullptr;
	void* _bufferClient = nullptr;

	IDPipe* _dpipeServer = nullptr;
	IDPipe* _dpipeClient = nullptr;

	BoolTrigger testEndTrigger;

	DWORD _blockReceivedCount = 0;
	chrono::nanoseconds duractionTotal{};
	size_t lineLen = 0;

	typedef std::chrono::high_resolution_clock Time;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader packet) {
		WriteServerLine() << "1. Client Connected" << END_LINE;
	}

	void ClientDisconnectCallback(IDPipe* pipe, PacketHeader packet) {
		WriteServerLine() << "2. Client Disconecting" << END_LINE;
	}

	wstring GetSpeed(long long bytes, std::chrono::nanoseconds duractionNS) {
		long double bytesF = bytes / 1.l;
		long double secF = duractionNS.count() / 1000000000.l;
		long double resultF = bytesF / secF;

		if (resultF > (1099511627776.l)) {
			long double res = resultF / 1099511627776.l;
			std::wstringstream ss;
			ss << res << L" Tb/s";
			return ss.str();
		}

		else if (resultF > 1073741824.l) {
			long double res = resultF / 1073741824.l;
			std::wstringstream ss;
			ss << res << L" Gb/s";
			return ss.str();
		}

		else if (resultF > 1048576.l) {
			long double res = resultF / 1048576.l;
			std::wstringstream ss;
			ss << res << L" Mb/s";
			return ss.str();
		}

		else if (resultF > 1024.l) {
			long double res = resultF / 1024.l;
			std::wstringstream ss;
			ss << res << L" Kb/s";
			return ss.str();
		}

		else {
			std::wstringstream ss;
			ss << resultF << L" b/s";
			return ss.str();
		}
	}

	wstring GetSize(long long bytes) {
		long double bytesF = bytes / 1.l;

		if (bytesF > 1099511627776.l) {
			long double res = bytesF / 1099511627776.l;
			std::wstringstream ss;
			ss << res << L" Tb";
			return ss.str();
		}

		else if (bytesF > 1073741824.l) {
			long double res = bytesF / 1073741824.l;
			std::wstringstream ss;
			ss << res << L" Gb";
			return ss.str();
		}

		else if (bytesF > 1048576.l) {
			long double res = bytesF / 1048576.l;
			std::wstringstream ss;
			ss << res << L" Mb";
			return ss.str();
		}

		else if (bytesF > 1024.l) {
			long double res = bytesF / 1024.l;
			std::wstringstream ss;
			ss << res << L" Kb";
			return ss.str();
		}

		else {
			std::wstringstream ss;
			ss << bytesF << L" b";
			return ss.str();
		}
	}

	void PacketHeaderRecevicedCallback(IDPipe* pipe, PacketHeader header) {

		DWORD dataSize = header.DataSize();

		if (dataSize != _bufferSize) {
			throw exception("Wrong data size");
		}

		auto readBeginTimePoint = Time::now();

		DWORD nBytesRead;

		bool bRet = _dpipeServer->Read(_bufferServer, dataSize, &nBytesRead, NULL);

		auto duraction = Time::now() - readBeginTimePoint;

		duractionTotal += duraction;

		if (newConsole) {
			wcout << wstring(lineLen, '\b') << flush;
			wstring line = L"Current pipe read speed: " + GetSpeed(dataSize, duraction);
			wcout << line;
			lineLen = line.length();
		}

		++_blockReceivedCount;

		if (_blockReceivedCount == _blockCount) {
			if (newConsole)
				wcout << endl;

			long long totalSize = (long long)_blockCount * (long long)_bufferSize;

			WriteServerLineW() << L"Total data size: " << GetSize(totalSize) << END_LINE;
			auto ms = chrono::duration_cast<chrono::milliseconds>(duractionTotal);
			WriteServerLineW() << L"Reading time: " << to_wstring(ms.count()) << L"ms" << END_LINE;
			wstring averageSpeed = GetSpeed(totalSize, duractionTotal);
			WriteServerLineW() << L"Average Pipe Read Speed: " << averageSpeed << END_LINE;

			testEndTrigger.SetComplete();
		}
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipeServer = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpipeServer->SetClientConnectCallback([this](IDPipe* pipe, PacketHeader packet) { ClientConnectCallback(pipe, packet); });
		_dpipeServer->SetPacketHeaderRecevicedCallback([this](IDPipe* pipe, PacketHeader packet) { PacketHeaderRecevicedCallback(pipe, packet); });
		_dpipeServer->SetOtherSideDisconnectCallback([this](IDPipe* pipe, PacketHeader packet) { ClientDisconnectCallback(pipe, packet); });

		auto blockSizeIt = params.flags.find(L"/s");
		if (blockSizeIt != params.flags.end())
			_bufferSize = stoi(blockSizeIt->second) * 1024 * 1024;

		auto blockCountIt = params.flags.find(L"/c");
		if (blockCountIt != params.flags.end())
			_blockCount = stoi(blockCountIt->second);

		_bufferServer = malloc(_bufferSize);
		_bufferClient = malloc(_bufferSize);

		_dpipeServer->Start();
		auto handleString = _dpipeServer->GetHandleString();

		_dpipeClient = DPipeBuilder::Create(handleString);
		_dpipeClient->Connect(handleString);

		for (DWORD i = 0; i < _blockCount; i++) {
			DWORD nBytesWritten;

			auto readBeginTimePoint = Time::now();
			_dpipeClient->Write(_bufferClient, _bufferSize, &nBytesWritten);
			chrono::nanoseconds duraction = Time::now() - readBeginTimePoint;

			duractionTotal += duraction;

			if (newConsole) {
				wcout << wstring(lineLen, '\b');
				wstring line = L"Current pipe write speed: " + GetSpeed(_bufferSize, duraction);
				wcout << line;
				lineLen = line.length();
			}
		}

		Sleep(500);
		long long totalSize = (long long)_blockCount * (long long)_bufferSize;
		auto ms = chrono::duration_cast<chrono::milliseconds>(duractionTotal);
		WriteClientLineW() << L"Write time: " << to_wstring(ms.count()) << L"ms" << END_LINE;
		wstring averageSpeed = GetSpeed(totalSize, duractionTotal);
		WriteClientLineW() << L"Average Pipe Write Speed: " << averageSpeed << END_LINE;

		wait(testEndTrigger);
		Sleep(1000);
		cout << endl;
	}
};

TestRegistrationServer ServerTestSpeedtest() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"SpeedTest";
	registration.title = L"Speed Test";
	registration.description = L"Description: Testing data transfering speed in raw IDPipe classes";
	registration.createHandler = [registration]() { return new ServerTestSpeedtestClass(registration); };
	return registration;
}