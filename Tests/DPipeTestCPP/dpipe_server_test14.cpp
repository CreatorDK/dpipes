#pragma once
#include "dpipe_server_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ServerTest14Class : public ServerTest {
public:
	ServerTest14Class(TestRegistrationServer registration) :
		ServerTest(registration) { }
private:

	IDPipe* _dpipe = nullptr;
	DPServer* _dpserver = nullptr;
	BoolTrigger lastRequestTrigger;
	BoolTrigger messagesRecivedTrigger;

	void ClientConnectCallback(IDPipe* pipe, PacketHeader header) {
		if (header.DataSize() > 0) {
			wstring message = _dpserver->GetWString(header);
			WriteServerLineW() << L"1. Client Connected with message: " << message << END_LINE;
		}
		else {
			WriteServerLineW() << L"1. Client Connected" << END_LINE;
		}
	}

	void ClientDisconnectCallback(IDPipe* pipe, PacketHeader header) {
		if (header.DataSize() > 0) {
			wstring message = _dpserver->GetWString(header);
			WriteServerLineW() << L"21. Client Disconnected with message: " << message << END_LINE;
		}
		else {
			WriteServerLineW() << L"21. Client Disconnected" << END_LINE;
		}
	}

	void OnInfoStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpserver->GetWString(header, data);

		if (message == L"Info string from client")
			WriteServerLineW() << L"2. Info string received" << END_LINE;
	}

	void OnInfoDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Info data from client")
			WriteServerLineW() << L"3. Info data received" << END_LINE;
	}

	void OnWarningStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpserver->GetWString(header, data);

		if (message == L"Warning string from client")
			WriteServerLineW() << L"4. Warning string received" << END_LINE;
	}

	void OnWarningDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Warning data from client")
			WriteServerLineW() << L"5. Warning data received" << END_LINE;
	}

	void OnErrorStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpserver->GetWString(header, data);

		if (message == L"Error string from client")
			WriteServerLineW() << L"6. Error string received" << END_LINE;
	}

	void OnErrorDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Error data from client")
			WriteServerLineW() << L"7. Error data received" << END_LINE;

		messagesRecivedTrigger.SetComplete();
	}

	void HandleCode1(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"14. Received request (handler 1): " << message << END_LINE;
		Sleep(3000);
		auto resp = req.createRespose();
		resp.code = 1;
		resp.dataType = 2;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode2(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"15. Received request (handler 2): " << message << END_LINE;
		Sleep(2000);
		auto resp = req.createRespose();
		resp.code = 2;
		resp.dataType = 3;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode3(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"16. Received request (handler 3): " << message << END_LINE;
		Sleep(1000);
		auto resp = req.createRespose();
		resp.code = 3;
		resp.dataType = 4;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode4(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"17. Received request (handler 4): " << message << END_LINE;
		Sleep(3500);
		auto resp = req.createRespose();
		resp.code = 4;
		resp.dataType = 5;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode5(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"18. Received request (handler 5): " << message << END_LINE;
		Sleep(2000);
		auto resp = req.createRespose();
		resp.code = 5;
		resp.dataType = 6;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode6(DPReceivedRequest& req) {
		wstring message((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		WriteServerLineW() << L"19. Received request (handler 6): " << message << END_LINE;
		Sleep(1000);
		auto resp = req.createRespose();
		resp.code = 6;
		resp.dataType = 7;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode7(DPReceivedRequest& req) {
		wstring threadId((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		if (newConsole)
			WriteServerLineW() << L"Received request (handler 7) sended sync from thread: " << threadId << END_LINE;
		auto resp = req.createRespose();
		resp.code = 8;
		resp.dataType = 9;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode8(DPReceivedRequest& req) {
		wstring threadId((wchar_t*)req.dataPtr(), req.dataSize() / sizeof(wchar_t));
		if (newConsole)
			WriteServerLineW() << L"Received request (handler 8) sended async from thread: " << threadId << END_LINE;
		auto resp = req.createRespose();
		resp.code = 10;
		resp.dataType = 11;
		_dpserver->SendResponse(req, resp);
	}

	void HandleCode9(DPReceivedRequest& req) {
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
		_dpserver->SetOnClientConnectCallback([this](IDPipe* pipe, PacketHeader header) { ClientConnectCallback(pipe, header); });
		_dpserver->SetOnClientDisconnectCallback([this](IDPipe* pipe, PacketHeader header) { ClientDisconnectCallback(pipe, header); });
		_dpserver->SetInfoStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnInfoStringReceived(pipe, header, data); });
		_dpserver->SetInfoDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnInfoDataReceived(pipe, header, data); });
		_dpserver->SetWarningStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnWarningStringReceived(pipe, header, data); });
		_dpserver->SetWarningDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnWarningDataReceived(pipe, header, data); });
		_dpserver->SetErrorStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnErrorStringReceived(pipe, header, data); });
		_dpserver->SetErrorDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnErrorDataReceived(pipe, header, data); });
		_dpserver->SetHandler(1, [this](DPReceivedRequest& req) {HandleCode1(req); });
		_dpserver->SetHandler(2, [this](DPReceivedRequest& req) {HandleCode2(req); });
		_dpserver->SetHandler(3, [this](DPReceivedRequest& req) {HandleCode3(req); });
		_dpserver->SetHandler(4, [this](DPReceivedRequest& req) {HandleCode4(req); });
		_dpserver->SetHandler(5, [this](DPReceivedRequest& req) {HandleCode5(req); });
		_dpserver->SetHandler(6, [this](DPReceivedRequest& req) {HandleCode6(req); });
		_dpserver->SetHandler(7, [this](DPReceivedRequest& req) {HandleCode7(req); });
		_dpserver->SetHandler(8, [this](DPReceivedRequest& req) {HandleCode8(req); });
		_dpserver->SetHandler(9, [this](DPReceivedRequest& req) {HandleCode9(req); });
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

TestRegistrationServer ServerTest14() {
	TestRegistrationServer registration;
	registration.enabled = true;
	registration.name = L"Test14";
	registration.title = L"DPServer - async, DPClient - async";
	registration.description = L"Description: Testing DPServer and DPClient communication";
	registration.createHandler = [registration]() { return new ServerTest14Class(registration); };
	return registration;
}