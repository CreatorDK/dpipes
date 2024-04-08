namespace CreatorDK.IO.DPipes.Win32
{
    public class PipeOpenMode
    {
        public const uint PIPE_ACCESS_DUPLEX                = 0x00000003;
        public const uint PIPE_ACCESS_INBOUND               = 0x00000001;
        public const uint PIPE_ACCESS_OUTBOUND              = 0x00000002;

        public const uint FILE_FLAG_FIRST_PIPE_INSTANCE     = 0x00080000;
        public const uint FILE_FLAG_WRITE_THROUGH           = 0x80000000;
        public const uint FILE_FLAG_OVERLAPPED              = 0x40000000;

        public const ulong WRITE_DAC                        = 0x00040000L;
        public const ulong WRITE_OWNER                      = 0x00080000L;
        public const ulong ACCESS_SYSTEM_SECURITY           = 0x01000000L;
    }
}
