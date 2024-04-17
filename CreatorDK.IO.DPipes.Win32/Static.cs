using CreatorDK.Diagnostics.Win32;
using Microsoft.Win32.SafeHandles;
using System;
using System.Runtime.InteropServices;

namespace CreatorDK.IO.DPipes.Win32
{
    public static class Static
    {
        public static void StartWin32 (this DPipe pipe) {
            if (pipe == null)
                throw new ArgumentNullException(nameof(pipe));

            else if (pipe is DPAnonymous pipeAnonymus)
            {
                SECURITY_ATTRIBUTES securityAttribute = new SECURITY_ATTRIBUTES()
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

                SafePipeHandle serverReadSafePipeHandle = new SafePipeHandle(serverReadPipeHandle, true);
                SafePipeHandle clientWriteSafePipeHandle = new SafePipeHandle(clientWritePipeHandle, true);
                SafePipeHandle serverWriteSafePipeHandle = new SafePipeHandle(serverWritePipeHandle, true);
                SafePipeHandle clientReadSafePipeHandle = new SafePipeHandle(clientReadPipeHandle, true);

                pipeAnonymus.Start(serverReadSafePipeHandle, clientWriteSafePipeHandle, serverWriteSafePipeHandle, clientReadSafePipeHandle);
            }

            else if (pipe.Type == DP_TYPE.NAMED_PIPE)
            {
                var namedPipe = (DPNamed)pipe;

                SECURITY_ATTRIBUTES securityAttribute = new SECURITY_ATTRIBUTES()
                {
                    bInheritHandle = 0
                };

                var serverReadPipeName = $"\\\\.\\pipe\\{pipe.Name}_read";

                IntPtr serverReadPipeHandle = NamedPipeWin32.CreateNamedPipeW(
                    serverReadPipeName,
                    (IntPtr)PipeOpenMode.PIPE_ACCESS_DUPLEX,
                    (IntPtr)PipeMode.PIPE_READMODE_BYTE,
                    (IntPtr)namedPipe.MaxInstances,
                    (IntPtr)namedPipe.OutBufferSize,
                    (IntPtr)namedPipe.InBufferSize,
                    (IntPtr)namedPipe.DefaultTimeout,
                    ref securityAttribute
                    );

                if ((uint)serverReadPipeHandle == HandleConstants.INVALID_HANDLE_VALUE)
                    throw new Exception($"Unable to create read named pipe. Code: {Marshal.GetLastWin32Error()}");

                var serverWritePipeName = $"\\\\.\\pipe\\{pipe.Name}_write";

                IntPtr serverWritePipeHandle = NamedPipeWin32.CreateNamedPipeW(
                    serverWritePipeName,
                    (IntPtr)PipeOpenMode.PIPE_ACCESS_DUPLEX,
                    (IntPtr)PipeMode.PIPE_TYPE_BYTE,
                    (IntPtr)namedPipe.MaxInstances,
                    (IntPtr)namedPipe.OutBufferSize,
                    (IntPtr)namedPipe.InBufferSize,
                    (IntPtr)namedPipe.DefaultTimeout,
                    ref securityAttribute
                    );

                if ((uint)serverWritePipeHandle == HandleConstants.INVALID_HANDLE_VALUE)
                    throw new Exception($"Unable to create write named pipe. Code: {Marshal.GetLastWin32Error()}");

                SafePipeHandle serverReadSafePipeHandle = new SafePipeHandle(serverReadPipeHandle, true);
                SafePipeHandle serverWriteSafePipeHandle = new SafePipeHandle(serverWritePipeHandle, true);

                namedPipe.Start(serverReadSafePipeHandle, serverWriteSafePipeHandle);
            }

            else
                throw new Exception("Unsupported pipe type");
        }
    }
}
