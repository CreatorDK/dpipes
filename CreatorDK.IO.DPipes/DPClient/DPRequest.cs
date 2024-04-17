namespace CreatorDK.IO.DPipes
{
    public struct DPRequest
    {
        public int Code {  get; set; }
        public int DataType { get; set; }
        public byte[] Data { get; set; }
        public int DescriptorType { get; set; }
        public int DescriptorSize { get; set; }
        public byte[] Descriptor { get; set; }
    }
}
