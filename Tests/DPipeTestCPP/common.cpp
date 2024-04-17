#include "common.h"

using namespace crdk::dpipes;
using namespace std;

void RunClientProccess(STARTUPINFO* si, PROCESS_INFORMATION* pi, start_params_server& params, wstring handleString, wstring test) {

    wstring path = params.path;

    wstring flags;

    for (const auto& pair : params.flags) {
        if (pair.first == L"/path" && pair.second.length() > 0) {
            path = pair.second;
            continue;
        }
        else if (pair.first == L"/attach_client") {
            flags = flags + L"/attach ";
            continue;
        }
        else if (pair.second.length() > 0) {
            flags = flags + pair.first + L" " + pair.second + L" ";
            continue;
        }
        else {
            flags = flags + pair.first + L" ";
        }
    }

    wstring newPath;
    newPath = L"\"" + newPath + L"\"";

    wstring commandLine = path + L" client " + handleString + L" " + test;

    if (flags.length() > 0)
        commandLine += (L" " + flags);

    LPWSTR lpCommand = &commandLine[0];

    DWORD creatinoFlag = 0;

    if (params.newConsole)
        creatinoFlag = CREATE_NEW_CONSOLE;

    if (!CreateProcess(&path[0],    // No module name (use command line)
        lpCommand,                  // Command line
        NULL,                       // Process handle not inheritable
        NULL,                       // Thread handle not inheritable
        TRUE,                       // Set handle inheritance
        creatinoFlag,               // No creation flags
        NULL,                       // Use parent's environment block
        NULL,                       // Use parent's starting directory 
        si,                         // Pointer to STARTUPINFO structure
        pi)                         // Pointer to PROCESS_INFORMATION structure
        )
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return;
    }
}

ServerTest::ServerTest(TestRegistrationServer registration) {
    _name = registration.name;
    _title = registration.title;
    _description = registration.description;
}

void ServerTest::ExecuteWrapper(start_params_server& params) {
    newConsole = params.newConsole;
    Execute(params);
}

void ServerTest::WriteTestName(const DP_TYPE type) const {
    if (type == DP_TYPE::ANONYMOUS_PIPE) 
        std::wcout << L"Anonymous Pipe Server" << std::endl;
    else 
        std::wcout << L"Named Pipe Server"  << std::endl;

    std::wcout << _name << L" (" << _title << ")" << std::endl;
    std::wcout << _description << std::endl;
}

ClientTest::ClientTest(TestRegistrationClient registration) {
    _name = registration.name;
    _title = registration.title;
    _description = registration.description;
}

void ClientTest::ExecuteWrapper(start_params_client& params) {
    newConsole = params.newConsole;
    Execute(params);
}

void ClientTest::WriteTestName(const DP_TYPE type) const {
    if (type == DP_TYPE::ANONYMOUS_PIPE) 
        std::wcout << L"Anonymous Pipe Client" << std::endl;
    else
        std::wcout << L"Named Pipe Client"  << std::endl;

    std::wcout << _name << " (" << _title << ")" << std::endl;
    std::wcout << _description << std::endl;
}

WriteServerLine::WriteServerLine() {
    wcout << L"CPP SRV: ";
}

WriteServerLine& WriteServerLine::operator<<(const std::string& arg)
{
    cout << arg;
    return *this;
}

WriteServerLine& WriteServerLine::operator<<(const char& arg)
{
    cout << arg;
    return *this;
}

WriteServerLineW::WriteServerLineW() {
    wcout << L"CPP SRV: ";
}

WriteServerLineW& WriteServerLineW::operator<<(const std::wstring& arg)
{
    wcout << arg;
    return *this;
}

WriteServerLineW& WriteServerLineW::operator<<(const wchar_t& arg)
{
    wcout << arg;
    return *this;
}

WriteClientLine::WriteClientLine() {
    wcout << L"CPP CL: ";
}

WriteClientLine& WriteClientLine::operator<<(const std::string& arg)
{
    cout << arg;
    return *this;
}

WriteClientLine& WriteClientLine::operator<<(const char& arg)
{
    cout << arg;
    return *this;
}

WriteClientLineW::WriteClientLineW() {
    wcout << L"CPP CL: ";
}

WriteClientLineW& WriteClientLineW::operator<<(const std::wstring& arg)
{
    wcout << arg;
    return *this;
}

WriteClientLineW& WriteClientLineW::operator<<(const wchar_t& arg)
{
    wcout << arg;
    return *this;
}