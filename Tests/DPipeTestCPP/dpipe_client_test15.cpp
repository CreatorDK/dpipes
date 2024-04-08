#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;

#define SPEEDTEST_DEFUALT_DATA_BLOC_SIZE 256*1024*1024;
#define SPEEDTEST_DEFUALT_DATA_BLOC_COUNT 20;

class ClientTest15Class : public ClientTest {
public:
	ClientTest15Class(TestRegistrationClient registration) :
		ClientTest(registration) {
	}
private:
	int _blockCount = SPEEDTEST_DEFUALT_DATA_BLOC_COUNT;
	DWORD _bufferSize = SPEEDTEST_DEFUALT_DATA_BLOC_SIZE;
	void* _buffer = nullptr;
	IDPipe* _dpipe = nullptr;

	DWORD _blockReceivedCount = 0;
	chrono::nanoseconds duractionTotal{};
	size_t lineLen = 0;

	typedef std::chrono::high_resolution_clock Time;

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

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		auto blockSizeIt = params.flags.find(L"/s");
		if (blockSizeIt != params.flags.end())
			_bufferSize = stoi(blockSizeIt->second) * 1024 * 1024;

		auto blockCountIt = params.flags.find(L"/c");
		if (blockCountIt != params.flags.end())
			_blockCount = stoi(blockCountIt->second);

		_buffer = malloc(_bufferSize);

		WriteClientLine() << "1. Connecting" << END_LINE;
		_dpipe->Connect(params.handle);

		for (int i = 0; i < _blockCount; i++) {
			DWORD nBytesWritten;

			auto readBeginTimePoint = Time::now();
			_dpipe->Write(_buffer, _bufferSize, &nBytesWritten);
			chrono::nanoseconds duraction = Time::now() - readBeginTimePoint;

			duractionTotal += duraction;

			if (newConsole) {
				wcout << wstring(lineLen, '\b');
				wstring line = L"Current pipe write speed: " + GetSpeed(_bufferSize, duraction);
				wcout << line;
				lineLen = line.length();
			}
		}

		if (newConsole)
		wcout << endl;

		Sleep(500);

		long long totalSize = (long long)_blockCount * (long long)_bufferSize;

		WriteClientLineW() << L"Total data size: " << GetSize(totalSize) << END_LINE;
		auto ms = chrono::duration_cast<chrono::milliseconds>(duractionTotal);
		WriteClientLineW() << L"Write time: " << to_wstring(ms.count()) << L"ms" << END_LINE;
		wstring averageSpeed = GetSpeed(totalSize, duractionTotal);
		WriteClientLineW() << L"Average Pipe Write Speed: " << averageSpeed << END_LINE;

		WriteClientLine() << "2. Disconecting" << END_LINE;
		_dpipe->Disconnect();

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest15() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test15";
	registration.title = L"Speed Test";
	registration.description = L"Description: Testing data transfering speed in raw IDPipe classes";
	registration.createHandler = [registration]() { return new ClientTest15Class(registration); };
	return registration;
}