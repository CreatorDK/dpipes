using CreatorDK.Diagnostics.Win32;
using System;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

namespace CreatorDK.IO.DPipes.Win32
{
    public partial class NamedPipeWin32
    {
        [DllImport("kernel32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto, SetLastError = true, BestFitMapping = false)]
        [ResourceExposure(ResourceScope.Process)]
        internal static extern bool CreatePipe(
            [In, Out]
            ref IntPtr readPipe,
            [In, Out]
            ref IntPtr writePipe,
            [In, Optional]
            ref SECURITY_ATTRIBUTES securityAttribute,
            [In]
            uint size
            );
    }
}
