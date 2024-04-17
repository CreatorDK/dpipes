#pragma once
#include "dpipe_client_tests.h"

using namespace std;
using namespace crdk::dpipes;
using namespace crdk::triggers;

class ClientTest14Class : public ClientTest {
public:
	ClientTest14Class(TestRegistrationClient registration) :
		ClientTest(registration),
		send10SyncRequestTrigger(10),
		send10AsyncRequestTrigger(10) {
		defaultString = L"DefaultString";
	}
private:

	IDPipe* _dpipe = nullptr;
	DPClient* _dpclient = nullptr;
	wstring defaultString;
	BoolTrigger messagesReceivedTrigger;
	BoolTrigger resp4ReceivedTrigger;
	BoolTrigger resp5ReceivedTrigger;
	BoolTrigger resp6ReceivedTrigger;
	IntTrigger send10SyncRequestTrigger;
	IntTrigger send10AsyncRequestTrigger;

	void RepsponseAsyncCallback(shared_ptr<DPResponse> resp) {
		WriteClientLineW() << L"Response On Async Request calback in thread " << END_LINE;
	}

	void OnInfoStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpclient->GetWString(header, data);

		if (message == L"Info string from server")
			WriteClientLineW() << L"8. Info string received" << END_LINE;
	}

	void OnInfoDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Info data from server")
			WriteClientLine() << "9. Info data received" << END_LINE;
	}

	void OnWarningStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpclient->GetWString(header, data);

		if (message == L"Warning string from server")
			WriteClientLine() << "10. Warning string received" << END_LINE;
	}

	void OnWarningDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Warning data from server")
			WriteClientLine() << "11. Warning data received" << END_LINE;
	}

	void OnErrorStringReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {

		wstring message = _dpclient->GetWString(header, data);

		if (message == L"Error string from server")
			WriteClientLine() << "12. Error string received" << END_LINE;
	}

	void OnErrorDataReceived(IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) {
		wstring message((wchar_t*)data->data(), data->size() / sizeof(wchar_t));

		if (message == L"Error data from server")
			WriteClientLine() << "13. Error data received" << END_LINE;

		messagesReceivedTrigger.SetComplete();
	}

	void Repsponse4Callback(shared_ptr<DPResponse> resp) {
		auto duraction = resp->totalDuraction();
		auto milliseconds = chrono::duration_cast<chrono::milliseconds>(duraction);
		WriteClientLineW() << L"17. Received response callback 4 in " << to_wstring(milliseconds.count()) << L"ms" << END_LINE;
		resp4ReceivedTrigger.SetComplete();
	}

	void Repsponse5Callback(shared_ptr<DPResponse> resp) {
		auto duraction = resp->totalDuraction();
		auto milliseconds = chrono::duration_cast<chrono::milliseconds>(duraction);
		WriteClientLineW() << L"18. Received response callback 5 in " << to_wstring(milliseconds.count()) << L"ms" << END_LINE;
		resp5ReceivedTrigger.SetComplete();
	}

	void Repsponse6Callback(shared_ptr<DPResponse> resp) {
		auto duraction = resp->totalDuraction();
		auto milliseconds = chrono::duration_cast<chrono::milliseconds>(duraction);
		WriteClientLineW() << L"19. Received response callback 6 in " << to_wstring(milliseconds.count()) << L"ms" << END_LINE;
		resp6ReceivedTrigger.SetComplete();
	}

	static void SendRequestSync(DPClient* dlclient, ClientTest14Class* test) {
		wstringstream ss;
		std::thread::id this_id = std::this_thread::get_id();
		ss << this_id;
		wstring threaIdString = ss.str();
		wstring message;
		message = L"Requset from " + threaIdString;
		DWORD len = boost::numeric_cast<DWORD>(message.length() * sizeof(wchar_t));
		auto resp = dlclient->SendRequest(7, 8, (void*)message.data(), len);
		if (test->newConsole)
			WriteClientLineW() << L"Response On Sync Request in thread " << threaIdString << END_LINE;
		test->send10SyncRequestTrigger.Increase(1);
	}

	void Send10RequestSync() {
		for (int i = 0; i < 10; i++) {
			thread th(SendRequestSync, _dpclient, this);
			th.detach();
		}
	}

	static void SendRequestAsync(DPClient* dlclient, ClientTest14Class* testClassPtr) {
		DWORD messageLen = boost::numeric_cast<DWORD>(testClassPtr->defaultString.length() * sizeof(wchar_t));
		if (testClassPtr->newConsole)
			dlclient->SendRequestAsync(8, 9, (void*)testClassPtr->defaultString.data(), messageLen, [testClassPtr](shared_ptr<DPResponse> resp) { testClassPtr->RepsponseAsyncCallback(resp); });
		else
			dlclient->SendRequestAsync(8, 9, (void*)testClassPtr->defaultString.data(), messageLen, nullptr);
		testClassPtr->send10AsyncRequestTrigger.Increase(1);
	}

	void Send10RequestAsync() {
		for (int i = 0; i < 10; i++) {
			thread th(SendRequestAsync, _dpclient, this);
			th.detach();
		}
	}

public:
	void Execute(start_params_client& params) override {
		_dpipe = DPipeBuilder::Create(params.handle);
		if (newConsole)
			WriteTestName(_dpipe->Type());

		_dpclient = new DPClient(_dpipe, true);
		_dpclient->SetInfoStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnInfoStringReceived(pipe, header, data); });
		_dpclient->SetInfoDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnInfoDataReceived(pipe, header, data); });
		_dpclient->SetWarningStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnWarningStringReceived(pipe, header, data); });
		_dpclient->SetWarningDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnWarningDataReceived(pipe, header, data); });
		_dpclient->SetErrorStringReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnErrorStringReceived(pipe, header, data); });
		_dpclient->SetErrorDataReceivedHandler([this](IDPipe* pipe, PacketHeader header, shared_ptr<HeapAllocatedData> data) { OnErrorDataReceived(pipe, header, data); });

		WriteClientLineW() << L"1. Connecting to server received" << END_LINE;
		_dpclient->Connect(params.handle, L"Hello, motherfucker!");

		WriteClientLineW() << L"2. Sending Info String" << END_LINE;
		_dpclient->SendInfo(L"Info string from client");

		WriteClientLineW() << L"3. Sending Info Data" << END_LINE;
		wstring infoData(L"Info data from client");
		DWORD infoDataLen = boost::numeric_cast<DWORD>(infoData.length() * sizeof(wchar_t));
		_dpclient->SendInfo((void*)infoData.data(), infoDataLen);

		WriteClientLineW() << L"4. Sending Warning String" << END_LINE;
		_dpclient->SendWarning(L"Warning string from client");

		WriteClientLineW() << L"5. Sending Warning Data" << END_LINE;
		wstring warningData(L"Warning data from client");
		DWORD warningDataLen = boost::numeric_cast<DWORD>(warningData.length() * sizeof(wchar_t));
		_dpclient->SendWarning((void*)warningData.data(), warningDataLen);

		WriteClientLineW() << L"6. Sending Error String" << END_LINE;
		_dpclient->SendError(L"Error string from client");

		WriteClientLineW() << L"7. Sending Error Data" << END_LINE;
		wstring errorData(L"Error data from client");
		DWORD errorDataLen = boost::numeric_cast<DWORD>(errorData.length() * sizeof(wchar_t));
		_dpclient->SendError((void*)errorData.data(), errorDataLen);

		//Waiting for incoming messages
		wait(messagesReceivedTrigger);

		wstring message1(L"Hello handler 1!");
		DWORD message1Len = boost::numeric_cast<DWORD>(message1.length() * sizeof(wchar_t));
		auto resp1 = _dpclient->SendRequest(1, 2, (void*)message1.data(), message1Len);
		auto duraction1 = resp1->totalDuraction();
		auto milliseconds1 = chrono::duration_cast<chrono::milliseconds>(duraction1);
		WriteClientLineW() << L"14. Received response 1 in " << to_wstring(milliseconds1.count()) << L"ms" << END_LINE;

		wstring message2(L"Hello handler 2!");
		DWORD message2Len = boost::numeric_cast<DWORD>(message2.length() * sizeof(wchar_t));
		auto resp2 = _dpclient->SendRequest(2, 3, (void*)message2.data(), message2Len);
		auto duraction2 = resp2->totalDuraction();
		auto milliseconds2 = chrono::duration_cast<chrono::milliseconds>(duraction2);
		WriteClientLineW() << L"15. Received response 2 in " << to_wstring(milliseconds2.count()) << L"ms" << END_LINE;

		wstring message3(L"Hello handler 3!");
		DWORD message3Len = boost::numeric_cast<DWORD>(message3.length() * sizeof(wchar_t));
		auto resp3 = _dpclient->SendRequest(3, 4, (void*)message3.data(), message3Len);
		auto duraction3 = resp3->totalDuraction();
		auto milliseconds3 = chrono::duration_cast<chrono::milliseconds>(duraction3);
		WriteClientLineW() << L"16. Received response 3 in " << to_wstring(milliseconds3.count()) << L"ms" << END_LINE;

		wstring message4(L"Hello handler 4!");
		DWORD message4Len = boost::numeric_cast<DWORD>(message4.length() * sizeof(wchar_t));
		auto callback4 = [this](shared_ptr<DPResponse> resp) { Repsponse4Callback(resp); };
		_dpclient->SendRequestAsync(4, 5, (void*)message4.data(), message4Len, callback4);

		wstring message5(L"Hello handler 5!");
		DWORD message5Len = boost::numeric_cast<DWORD>(message5.length() * sizeof(wchar_t));
		auto callback5 = [this](shared_ptr<DPResponse> resp) { Repsponse5Callback(resp); };
		_dpclient->SendRequestAsync(5, 6, (void*)message5.data(), message5Len, callback5);

		wstring message6(L"Hello handler 6!");
		DWORD message6Len = boost::numeric_cast<DWORD>(message6.length() * sizeof(wchar_t));
		auto callback6 = [this](shared_ptr<DPResponse> resp) { Repsponse6Callback(resp); };
		_dpclient->SendRequestAsync(6, 7, (void*)message6.data(), message6Len, callback6);

		Send10RequestSync();
		wait(send10SyncRequestTrigger);

		Send10RequestAsync();
		wait(send10AsyncRequestTrigger);

		wait(resp4ReceivedTrigger);
		wait(resp5ReceivedTrigger);
		wait(resp6ReceivedTrigger);

		WriteClientLineW() << L"20. Sending last request" << END_LINE;
		wstring messagelast(L"Hello last handler!");
		DWORD messagelastLen = boost::numeric_cast<DWORD>(messagelast.length() * sizeof(wchar_t));
		_dpclient->SendRequest(9, 10, (void*)messagelast.data(), messagelastLen);

		WriteClientLineW() << L"21. Disconnecting" << END_LINE;
		wstring disconnectMessage(L"Goodbuy, motherfucker!");
		DWORD disconnectMessageLen = boost::numeric_cast<DWORD>(disconnectMessage.length() * sizeof(wchar_t));
		_dpclient->Disconnect((void*)disconnectMessage.data(), disconnectMessageLen);

		if (newConsole)
			system("pause");
	}
};

TestRegistrationClient ClientTest14() {
	TestRegistrationClient registration;
	registration.enabled = true;
	registration.name = L"Test14";
	registration.title = L"DPServer - async, DPClient - async";
	registration.description = L"Description: Testing DPServer and DPClient communication";
	registration.createHandler = [registration]() { return new ClientTest14Class(registration); };
	return registration;
}