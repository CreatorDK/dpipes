#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest13Class : public ServerTest {
public:
	ServerTest13Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	DPServer* _dpserver = nullptr;
	BoolTrigger lastRequestTrigger;
	BoolTrigger messagesRecivedTrigger;

	void ClientConnectCallback(shared_ptr<HeapAllocatedData> heapData) {
		if (heapData->size() > 0) {
			wstring message((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
			WriteServerLineW() << L"1. Client Connected with message: " << message << END_LINE;
		}
		else {
			WriteServerLineW() << L"1. Client Connected" << END_LINE;
		}
	}

	void ClientDisconnectCallback(shared_ptr<HeapAllocatedData> heapData) {
		if (heapData->size() > 0) {
			wstring message((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));
			WriteServerLineW() << L"21. Client Disconnected with message: " << message << END_LINE;
		}
		else {
			WriteServerLineW() << L"21. Client Disconnected" << END_LINE;
		}
	}

	void OnInfoStringReceived(wstring infoString) {
		if (infoString == L"Info string from client")
			WriteServerLineW() << L"2. Info string received" << END_LINE;
	}

	void OnInfoDataReceived(shared_ptr<HeapAllocatedData> heapData) {
		wstring message((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));

		if (message == L"Info data from client")
			WriteServerLineW() << L"3. Info data received" << END_LINE;
	}

	void OnWarningStringReceived(wstring warningString) {
		if (warningString == L"Warning string from client")
			WriteServerLineW() << L"4. Warning string received" << END_LINE;
	}

	void OnWarningDataReceived(shared_ptr<HeapAllocatedData> heapData) {
		wstring message((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));

		if (message == L"Warning data from client")
			WriteServerLineW() << L"5. Warning data received" << END_LINE;
	}

	void OnErrorStringReceived(wstring errorString) {
		if (errorString == L"Error string from client")
			WriteServerLineW() << L"6. Error string received" << END_LINE;
	}

	void OnErrorDataReceived(shared_ptr<HeapAllocatedData> heapData) {
		wstring message((wchar_t*)heapData->data(), heapData->size() / sizeof(wchar_t));

		if (message == L"Error data from client")
			WriteServerLineW() << L"7. Error data received" << END_LINE;

		messagesRecivedTrigger.SetComplete();
	}

	void HandleCode1(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"14. Received request (handler 1): " << message << END_LINE;
		Sleep(3000);
		auto resp = req.createRespose();
		resp.code = 1;
		resp.dataType = 2;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode2(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"15. Received request (handler 2): " << message << END_LINE;
		Sleep(2000);
		auto resp = req.createRespose();
		resp.code = 2;
		resp.dataType = 3;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode3(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"16. Received request (handler 3): " << message << END_LINE;
		Sleep(1000);
		auto resp = req.createRespose();
		resp.code = 3;
		resp.dataType = 4;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode4(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"17. Received request (handler 4): " << message << END_LINE;
		Sleep(3500);
		auto resp = req.createRespose();
		resp.code = 4;
		resp.dataType = 5;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode5(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"18. Received request (handler 5): " << message << END_LINE;
		Sleep(2000);
		auto resp = req.createRespose();
		resp.code = 5;
		resp.dataType = 6;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode6(DPClientRequest& req) {
		wstring message((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"19. Received request (handler 6): " << message << END_LINE;
		Sleep(1000);
		auto resp = req.createRespose();
		resp.code = 6;
		resp.dataType = 7;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode7(DPClientRequest& req) {
		wstring threadId((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		if (newConsole)
			WriteServerLineW() << L"Received request (handler 7) sended sync from thread: " << threadId << END_LINE;
		auto resp = req.createRespose();
		resp.code = 8;
		resp.dataType = 9;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode8(DPClientRequest& req) {
		wstring threadId((wchar_t*)req.data(), req.dataSize() / sizeof(wchar_t));
		if (newConsole)
			WriteServerLineW() << L"Received request (handler 8) sended async from thread: " << threadId << END_LINE;
		auto resp = req.createRespose();
		resp.code = 10;
		resp.dataType = 11;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode9(DPClientRequest& req) {
		WriteServerLineW() << L"20. Received last request (handler 9)" << END_LINE;
		auto resp = req.createRespose();
		resp.code = 11;
		resp.dataType = 12;
		_dpserver->SendResponse(req, resp);
		lastRequestTrigger.SetComplete();
	}

public:
	void Execute(start_params_server& params) override {
		WriteTestName(params.pipeType);
		_dpipe = DPipeBuilder::Create(params.pipeType, L"\\\\.\\pipe\\test-pipe-123");

		_dpserver = new DPServer(_dpipe, true);
		_dpserver->SetOnClientConnectCallback([this](shared_ptr<HeapAllocatedData> data) { ClientConnectCallback(data); });
		_dpserver->SetOnClientDisconnectCallback([this](shared_ptr<HeapAllocatedData> data) { ClientDisconnectCallback(data); });
		_dpserver->SetInfoStringReceivedCallback([this](wstring infoString) { OnInfoStringReceived(infoString); });
		_dpserver->SetInfoDataReceivedCallback([this](shared_ptr<HeapAllocatedData> heapData) { OnInfoDataReceived(heapData); });
		_dpserver->SetWarningStringReceivedCallback([this](wstring warningString) { OnWarningStringReceived(warningString); });
		_dpserver->SetWarningDataReceivedCallback([this](shared_ptr<HeapAllocatedData> heapData) { OnWarningDataReceived(heapData); });
		_dpserver->SetErrorStringReceivedCallback([this](wstring errorString) { OnErrorStringReceived(errorString); });
		_dpserver->SetErrorDataReceivedCallback([this](shared_ptr<HeapAllocatedData> heapData) { OnErrorDataReceived(heapData); });
		_dpserver->SetHandler(1, [this](DPClientRequest& req) {HandleCode1(req); });
		_dpserver->SetHandler(2, [this](DPClientRequest& req) {HandleCode2(req); });
		_dpserver->SetHandler(3, [this](DPClientRequest& req) {HandleCode3(req); });
		_dpserver->SetHandler(4, [this](DPClientRequest& req) {HandleCode4(req); });
		_dpserver->SetHandler(5, [this](DPClientRequest& req) {HandleCode5(req); });
		_dpserver->SetHandler(6, [this](DPClientRequest& req) {HandleCode6(req); });
		_dpserver->SetHandler(7, [this](DPClientRequest& req) {HandleCode7(req); });
		_dpserver->SetHandler(8, [this](DPClientRequest& req) {HandleCode8(req); });
		_dpserver->SetHandler(9, [this](DPClientRequest& req) {HandleCode9(req); });
		_dpserver->Start();
		auto hadnle = _dpserver->GetHandle();

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		RunClientProccess(&si, &pi, params, hadnle.get()->AsString(), _name);

		//Waiting for incoming messages
		wait(messagesRecivedTrigger);

		WriteServerLineW() << L"8. Sending Info String" << END_LINE;
		_dpserver->SendInfo(L"Info string from server");

		WriteServerLineW() << L"9. Sending Info Data" << END_LINE;
		wstring infoData(L"Info data from server");
		DWORD infoDataLen = boost::numeric_cast<DWORD>(infoData.length() * sizeof(wchar_t));
		_dpserver->SendInfo((void*)infoData.data(), infoDataLen);

		WriteServerLineW() << L"10. Sending Warning String" << END_LINE;
		_dpserver->SendWarning(L"Warning string from server");

		WriteServerLineW() << L"11. Sending Warning Data" << END_LINE;
		wstring warningData(L"Warning data from server");
		DWORD warningDataLen = boost::numeric_cast<DWORD>(warningData.length() * sizeof(wchar_t));
		_dpserver->SendWarning((void*)warningData.data(), warningDataLen);

		WriteServerLineW() << L"12. Sending Error String" << END_LINE;
		_dpserver->SendError(L"Error string from server");

		WriteServerLineW() << L"13. Sending Error Data" << END_LINE;
		wstring errorData(L"Error data from server");
		DWORD errorDataLen = boost::numeric_cast<DWORD>(errorData.length() * sizeof(wchar_t));
		_dpserver->SendError((void*)errorData.data(), errorDataLen);

		wait(lastRequestTrigger);

		Sleep(500);
		std::cout << std::endl;
	}
};

TestRegistrationServer ServerTest13() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test13";
	registration.title = L"DPServer - async, DPClient - sync";
	registration.description = L"Description: Testing DPServer and DPClient communication";
	registration.createHandler = [registration]() { return new ServerTest13Class(registration); };
	return registration;
}