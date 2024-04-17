#include "heapdata.h"

using namespace std;

//HeapAllocatedData implementation
#pragma region HeapAllocatedData

shared_mutex HeapAllocatedData::_mutex;
DWORD HeapAllocatedData::_memoryAllocated;
function<void(void* address, DWORD size)> HeapAllocatedData::_onMemoryAllocatedCallback;
function<void(void* address, DWORD size)> HeapAllocatedData::_onMemoryDeallocatedCallback;
list<pair<void*, DWORD>> HeapAllocatedData::_allocatedAddressList;

HeapAllocatedData::HeapAllocatedData(void* data, DWORD size, bool setkeepAllocatedMemory) {
	_data = data;
	_size = size;
	keepAllocatedMemory = setkeepAllocatedMemory;
}

HeapAllocatedData::HeapAllocatedData(DWORD size, bool setkeepAllocatedMemory) {

	if (size > 0) {
		_data = new char[size];
		_size = size;

		OnMemoryAllocatedInner(_data, _size);
	}

	_dataOwner = true;
	keepAllocatedMemory = setkeepAllocatedMemory;
}

HeapAllocatedData::~HeapAllocatedData() {

	if (!keepAllocatedMemory && _data != nullptr && _dataOwner) {
		delete[] _data;

		OnMemoryDeallocatedInner(_data, _size);

	}
	else if (_size > 0) {
		_allocatedAddressList.push_front(std::make_pair(_data, _size));
	}
}

void HeapAllocatedData::OnMemoryAllocationChanged(LPVOID dataPtr) {
	auto memoryData = (MemoryDataCallback*)dataPtr;

	if (memoryData == nullptr)
		return;

	memoryData->callback(memoryData->address, memoryData->size);

	delete memoryData;
}

void HeapAllocatedData::OnMemoryAllocatedInner(void* address, DWORD size) {

	if (size > 0) {

		_memoryAllocated = _memoryAllocated + size;

		if (_onMemoryAllocatedCallback) {
			MemoryDataCallback* md = new MemoryDataCallback(address, size, _onMemoryAllocatedCallback);
			DWORD _nReadThreadId;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMemoryAllocationChanged, (LPVOID)md, 0, &_nReadThreadId);
		}
	}
}

void HeapAllocatedData::OnMemoryDeallocatedInner(void* address, DWORD size) {

	if (size > 0) {

		_memoryAllocated = _memoryAllocated - size;

		if (_onMemoryDeallocatedCallback) {
			MemoryDataCallback* md = new MemoryDataCallback(address, size, _onMemoryDeallocatedCallback);
			DWORD _nReadThreadId;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnMemoryAllocationChanged, (LPVOID)md, 0, &_nReadThreadId);
		}
	}
}

void HeapAllocatedData::SetMemoryAllocatedCallback(std::function<void(void* address, DWORD size)> function) {
	_onMemoryAllocatedCallback = function;
}

void HeapAllocatedData::SetMemoryDeallocatedCallback(std::function<void(void* address, DWORD size)> function) {
	_onMemoryDeallocatedCallback = function;
}

DWORD HeapAllocatedData::GetMemoryAllocated() {
	return _memoryAllocated;
}

DWORD HeapAllocatedData::GetMemoryAllocatedMB() {
	DWORD result;
	_mutex.lock();
	result = _memoryAllocated / 1048576;
	_mutex.unlock();
	return result;
}

bool HeapAllocatedData::CanAllocate(DWORD dataSize, DWORD limit) {
	bool result;
	_mutex.lock();
	DWORD total = dataSize + _memoryAllocated;
	result = total <= limit;
	_mutex.unlock();
	return result;
}

void HeapAllocatedData::Free(void* buffer) {
	_mutex.lock();

	auto it = _allocatedAddressList.begin();

	for (; it != _allocatedAddressList.end(); ) {
		if (it->first == buffer) {
			delete[] it->first;
			OnMemoryDeallocatedInner(it->first, it->second);
			it = _allocatedAddressList.erase(it);
		}
		else {
			++it;
		}
	}
	_mutex.unlock();
}

void HeapAllocatedData::Free(unsigned long long dataAddress) {
	_mutex.lock();

	auto it = _allocatedAddressList.begin();

	for (; it != _allocatedAddressList.end(); ) {
		if (((unsigned long long)it->first) == dataAddress) {
			delete[] it->first;
			OnMemoryDeallocatedInner(it->first, it->second);
			it = _allocatedAddressList.erase(it);
		}
		else {
			++it;
		}
	}
	_mutex.unlock();
}

void HeapAllocatedData::Free() {
	_mutex.lock();

	auto it = _allocatedAddressList.begin();

	while (it != _allocatedAddressList.end()) {
		delete[] it->first;
		OnMemoryDeallocatedInner(it->first, it->second);
		it = _allocatedAddressList.erase(it);
	}

	_mutex.unlock();
}

void HeapAllocatedData::ReadAllocatedAddresses(list<pair<void*, DWORD>>& list)
{
	_mutex.lock();
	std::copy(_allocatedAddressList.begin(), _allocatedAddressList.end(), std::front_inserter(list));
	_mutex.unlock();
}

string HeapAllocatedData::GetSize(unsigned long long bytes) {

	long double bytesF = bytes / 1.l;

	if (bytesF > 1099511627776.l) {
		long double res = bytesF / 1099511627776.l;
		std::stringstream ss;
		ss << res << " Tb";
		return ss.str();
	}

	else if (bytesF > 1073741824.l) {
		long double res = bytesF / 1073741824.l;
		std::stringstream ss;
		ss << res << " Gb";
		return ss.str();
	}

	else if (bytesF > 1048576.l) {
		long double res = bytesF / 1048576.l;
		std::stringstream ss;
		ss << res << " Mb";
		return ss.str();
	}

	else if (bytesF > 1024.l) {
		long double res = bytesF / 1024.l;
		std::stringstream ss;
		ss << res << " Kb";
		return ss.str();
	}

	else {
		std::stringstream ss;
		ss << bytesF << " b";
		return ss.str();
	}
}

string HeapAllocatedData::GetSize() {
	return GetSize(_memoryAllocated);
}

wstring HeapAllocatedData::GetSizeW(unsigned long long bytes) {

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

wstring HeapAllocatedData::GetSizeW() {
	return GetSizeW(_memoryAllocated);
}

void* HeapAllocatedData::data() const {
	return _data;
}

DWORD HeapAllocatedData::size() const {
	return _size;
}
#pragma endregion HeapAllocatedData