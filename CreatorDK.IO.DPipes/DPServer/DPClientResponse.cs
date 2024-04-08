namespace CreatorDK.IO.DPipes
{
    public struct DPClientResponse
    {
        public Guid Guid { get; set; }
        public int Code { get; set; }
        public int DataType { get; set; }
        public byte[]? Data { get; set; }
    }
}
