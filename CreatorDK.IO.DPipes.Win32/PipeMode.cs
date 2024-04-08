using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes.Win32
{
    public class PipeMode
    {
        public const uint PIPE_TYPE_BYTE            = 0x00000000;
        public const uint PIPE_TYPE_MESSAGE         = 0x00000004;

        public const uint PIPE_READMODE_BYTE        = 0x00000000;
        public const uint PIPE_READMODE_MESSAGE     = 0x00000002;

        public const uint PIPE_WAIT                 = 0x00000000;
        public const uint PIPE_NOWAIT               = 0x00000001;

        public const uint PIPE_ACCEPT_REMOTE_CLIENTS = 0x00000000;
        public const uint PIPE_REJECT_REMOTE_CLIENTS = 0x00000008;
    }
}
