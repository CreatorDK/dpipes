#include <windows.h>
#include <functional>
#include <shared_mutex>
#include <sstream>
#include <list>

struct MemoryDataCallback {
	void* address;
	DWORD size;
	std::function<void(void* address, DWORD size)> callback;
	MemoryDataCallback(void* dataAddress, DWORD dataSize, std::function<void(void* address, DWORD size)> callbackFunction);
};

//Struct contains buffer adress and size. Depending on flag keepAllocatedMemory at destructor free or not free allocated memory
#pragma region HeapAllocatedData
struct HeapAllocatedData {
public:
	HeapAllocatedData(void* data, DWORD size, bool keepAllocatedMemory = false);
	HeapAllocatedData(DWORD size, bool keepAllocatedMemory = false);
	~HeapAllocatedData();
	HeapAllocatedData(const HeapAllocatedData&) = delete;
	HeapAllocatedData operator=(const HeapAllocatedData& obj) = delete;
private:
	void* _data = nullptr;
	DWORD _size = 0;
	bool _dataOwner = false;
private:
	static void OnMemoryAllocationChanged(LPVOID dataPtr);
	static std::function<void(void* address, DWORD size)> _onMemoryAllocatedCallback;
	static std::function<void(void* address, DWORD size)> _onMemoryDeallocatedCallback;
	static void OnMemoryAllocatedInner(void* address, DWORD size);
	static void OnMemoryDeallocatedInner(void* address, DWORD size);
	static std::shared_mutex _mutex;
	static DWORD _memoryAllocated;
	static std::list<std::pair<void*, DWORD>> _allocatedAddressList;
public:
	static void SetMemoryAllocatedCallback(std::function<void(void* address, DWORD size)> function);
	static void SetMemoryDeallocatedCallback(std::function<void(void* address, DWORD size)> function);
	static DWORD GetMemoryAllocated();
	static DWORD GetMemoryAllocatedMB();
	static bool CanAllocate(DWORD dataSize, DWORD limit);
	static void Free(unsigned long long dataAddress);
	static void Free(void* buffer);
	static void Free();
	static void ReadAllocatedAddresses(std::list<std::pair<void*, DWORD>>& list);
	static std::string GetSize(unsigned long long bytes);
	static std::string GetSize();
	static std::wstring GetSizeW(unsigned long long bytes);
	static std::wstring GetSizeW();
public:
	bool keepAllocatedMemory = false;
	void* data() const;
	DWORD size() const;
};
#pragma endregion HeapAllocatedData