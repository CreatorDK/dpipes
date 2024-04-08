using CreatorDK.Diagnostics.Win32;
using Microsoft.Win32.SafeHandles;
using System.Runtime.InteropServices;

namespace CreatorDK.IO.DPipes.Win32
{
    public static class Static
    {
        public static void StartWin32 (this DPipe pipe) {
            if (pipe == null)
                throw new ArgumentNullException(nameof(pipe));

            else if (pipe is DPipeAnonymus pipeAnonymus)
            {
                SECURITY_ATTRIBUTES securityAttribute = new()
                {
                    bInheritHandle = 1
                };

                IntPtr serverReadPipeHandle = IntPtr.Zero;
                IntPtr clientWritePipeHandle = IntPtr.Zero;

                bool result = NamedPipeWin32.CreatePipe(ref serverReadPipeHandle, ref clientWritePipeHandle, ref securityAttribute, (uint)pipe.InBufferSize);

                if (!result)
                    throw new Exception("Unable to create read anonymus pipe");

                HandleWin32.SetHandleInformation(serverReadPipeHandle, HandleFlags.HANDLE_FLAG_INHERIT, 0);

                IntPtr serverWritePipeHandle = IntPtr.Zero;
                IntPtr clientReadPipeHandle = IntPtr.Zero;

                result = NamedPipeWin32.CreatePipe(ref clientReadPipeHandle, ref serverWritePipeHandle, ref securityAttribute, (uint)pipe.OutBufferSize);

                if (!result)
                {
                    HandleWin32.CloseHandle(serverReadPipeHandle);
                    HandleWin32.CloseHandle(clientWritePipeHandle);
                    throw new Exception("Unable to create read anonymus pipe");
                }

                SafePipeHandle serverReadSafePipeHandle = new(serverReadPipeHandle, true);
                SafePipeHandle clientWriteSafePipeHandle = new(clientWritePipeHandle, true);
                SafePipeHandle serverWriteSafePipeHandle = new(serverWritePipeHandle, true);
                SafePipeHandle clientReadSafePipeHandle = new(clientReadPipeHandle, true);

                pipeAnonymus.Start(serverReadSafePipeHandle, clientWriteSafePipeHandle, serverWriteSafePipeHandle, clientReadSafePipeHandle);
            }

            else if (pipe.Type == DPIPE_TYPE.NAMED_PIPE)
            {
                var namedPipe = (DPipeNamed)pipe;

                SECURITY_ATTRIBUTES securityAttribute = new()
                {
                    bInheritHandle = 0
                };

                var serverReadPipeName = $"\\\\.\\pipe\\{pipe.Name}_read";

                IntPtr serverReadPipeHandle = NamedPipeWin32.CreateNamedPipeW(
                    serverReadPipeName,
                    (nint)PipeOpenMode.PIPE_ACCESS_DUPLEX,
                    (nint)PipeMode.PIPE_READMODE_BYTE,
                    namedPipe.MaxInstances,
                    namedPipe.OutBufferSize,
                    namedPipe.InBufferSize,
                    namedPipe.DefaultTimeout,
                    ref securityAttribute
                    );

                if ((uint)serverReadPipeHandle == HandleConstants.INVALID_HANDLE_VALUE)
                    throw new Exception($"Unable to create read named pipe. Code: {Marshal.GetLastWin32Error()}");

                var serverWritePipeName = $"\\\\.\\pipe\\{pipe.Name}_write";

                IntPtr serverWritePipeHandle = NamedPipeWin32.CreateNamedPipeW(
                    serverWritePipeName,
                    (nint)PipeOpenMode.PIPE_ACCESS_DUPLEX,
                    (nint)PipeMode.PIPE_TYPE_BYTE,
                    namedPipe.MaxInstances,
                    namedPipe.OutBufferSize,
                    namedPipe.InBufferSize,
                    namedPipe.DefaultTimeout,
                    ref securityAttribute
                    );

                if ((uint)serverWritePipeHandle == HandleConstants.INVALID_HANDLE_VALUE)
                    throw new Exception($"Unable to create write named pipe. Code: {Marshal.GetLastWin32Error()}");

                SafePipeHandle serverReadSafePipeHandle = new(serverReadPipeHandle, true);
                SafePipeHandle serverWriteSafePipeHandle = new(serverWritePipeHandle, true);

                namedPipe.Start(serverReadSafePipeHandle, serverWriteSafePipeHandle);
            }

            else
                throw new Exception("Unsupported pipe type");
        }
    }
}
