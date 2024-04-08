dpipes-dotnet-cpp-win32
=======
DPipes is abstract classes (both for C++ and .NET applications) to IPC communications for 4 scenarios:
Inniciator - Client:
1) .NET app - .NET app
2) C++ app - C++ app
3) .NET app - C++ app
4) C++ app - .NET app

DPipes work over Anonymous and Named pipes. To choose which pipe to use you need pass type in fabric method of PipeBuilder class
After DPipe instance created Start() method should be called (its creates pipe and keep handles of each end of pipe)
After DPipe instance started it's possible to get its DPipeHandle instance (it can be Anonymous that keep 2 handles to read and write pipes or Named that keep Named pipe name)
DPipe handle has method AsString() that returns string representation of handle
Client can connect to the pipe using this string (its automatically detect which pipe to use on string syntax)