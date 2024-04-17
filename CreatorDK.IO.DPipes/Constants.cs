namespace CreatorDK.IO.DPipes
{
    static class Constants
    {
        public const uint DP_PACKET_HEADER_SIZE                 = 8;
        public const uint DP_BUFFER_SIZE_DEFAULT                = 0;

        public const byte DP_SERVICE_CODE_RAW_CLIENT            = 0;
        public const byte DP_SERVICE_CODE_CONNECT               = 1;
        public const byte DP_SERVICE_CODE_DISCONNECT            = 2;
        public const byte DP_SERVICE_CODE_DISCONNECTED          = 3;
        public const byte DP_SERVICE_CODE_TERMINATING           = 4;
        public const byte DP_SERVICE_CODE_PING                  = 5;
        public const byte DP_SERVICE_CODE_PONG                  = 6;
        public const byte DP_SERVICE_CODE_MTU_REQUEST           = 7;
        public const byte DP_SERVICE_CODE_MTU_RESPONSE          = 8;
        public const byte DP_SERVICE_CODE_SEND_CONFIGURATION    = 9;

        public const uint DP_INFO_DATA                          = 10;
        public const uint DP_INFO_STRING                        = 11;
        public const uint DP_WARNING_DATA                       = 20;
        public const uint DP_WARNING_STRING                     = 21;
        public const uint DP_ERROR_DATA                         = 30;
        public const uint DP_ERROR_STRING                       = 31;
        public const uint DP_MESSAGE_DATA                       = 40;
        public const uint DP_MESSAGE_STRING                     = 41;

        public const uint DP_REQUEST                            = 64;
        public const uint DP_RESPONSE = 128;

        public const uint DP_REQUEST_SIZE                       = 32;
        public const uint DP_RESPONSE_SIZE                      = 32;

        public const uint DP_DATA_BINARY                        = 0;
        public const uint DP_DATA_STRING                        = 1;
        public const uint DP_DATA_WSTRING                       = 2;
        public const uint DP_DATA_STRING_JSON                   = 3;
        public const uint DP_DATA_WSRTING_JSON                  = 4;

        public const uint DP_HANDLER_NOT_FOUND                  = 404;
        public const uint DP_REQUEST_TIMEOUT                    = 408;

        public const uint DP_ENCODING_MASK                      = 2139095040;
        public const uint DP_ENCODING_UTF8		                = 0;
        public const uint DP_ENCODING_UNICODE		            = 1;
    }
}
