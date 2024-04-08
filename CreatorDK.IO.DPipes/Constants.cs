namespace CreatorDK.IO.DPipes
{
    static class Constants
    {
        public const uint PACKET_HEADER_SIZE             = 8;
        public const uint PIPE_BUFFERLENGTH_DEFAULT      = 0;
        public const uint SERVICE_CODE_RAW_CLIENT       = 0;
        public const uint SERVICE_CODE_CONNECT          = 1;
        public const uint SERVICE_CODE_DISCONNECT       = 2;
        public const uint SERVICE_CODE_DISCONNECTED     = 3;
        public const uint SERVICE_CODE_TERMINATING      = 4294967295;

        public const uint DP_REQUEST                    = 0;
        public const uint DP_RESPONSE                   = 0;
        public const uint DP_INFO_DATA                  = 10;
        public const uint DP_INFO_STRING                = 11;
        public const uint DP_WARNING_DATA               = 20;
        public const uint DP_WARNING_STRING             = 21;
        public const uint DP_ERROR_DATA                 = 30;
        public const uint DP_ERROR_STRING               = 31;
        public const uint DP_MESSAGE_DATA               = 40;
        public const uint DP_MESSAGE_STRING             = 41;

        public const uint DP_REQUEST_SIZE               = 24;
        public const uint DP_RESPONSE_SIZE              = 24;

        public const uint DP_DATA_BINARY                = 0;
        public const uint DP_DATA_STRING                = 1;
        public const uint DP_DATA_WSTRING               = 2;
        public const uint DP_DATA_STRING_JSON           = 3;
        public const uint DP_DATA_WSRTING_JSON          = 4;

        public const uint DP_HANDLER_NOT_FOUND          = 404;
        public const uint DP_REQUEST_TIMEOUT            = 408;
    }
}
