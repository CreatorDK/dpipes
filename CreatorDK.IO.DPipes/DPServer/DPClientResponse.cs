using System;

namespace CreatorDK.IO.DPipes
{
    public struct DPClientResponse
    {
        public Guid Guid { get; set; }
        public int Code { get; set; }
        public int DataType { get; set; }
        public int DescriptionType { get; set; }
        public int DescriptionSize { get; set; }
        public byte[] Data { get; set; }
        public byte[] Description { get; set; }
    }
}
