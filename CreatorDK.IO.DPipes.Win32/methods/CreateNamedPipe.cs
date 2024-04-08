using CreatorDK.Diagnostics.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Text;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes.Win32
{
    public partial class NamedPipeWin32
    {
        [DllImport("kernel32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto, SetLastError = true, BestFitMapping = false)]
        [ResourceExposure(ResourceScope.Process)]
        internal static extern IntPtr CreateNamedPipeW(
            [In, MarshalAs(UnmanagedType.LPTStr)]
            string lpName,
            [In]
            IntPtr dwOpenMode,
            [In]
            IntPtr dwPipeMode,
            [In]
            IntPtr nMaxInstances,
            [In]
            IntPtr nOutBufferSize,
            [In]
            IntPtr nInBufferSize,
            [In]
            IntPtr nDefaultTimeOut,
            [In, Optional]
            ref SECURITY_ATTRIBUTES securityAttribute
            );
    }
}
