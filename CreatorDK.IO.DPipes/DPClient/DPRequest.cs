namespace CreatorDK.IO.DPipes
{
    public struct DPRequest
    {
        public int Code {  get; set; }
        public int DataType { get; set; }
        public byte[]? Data { get; set; }
    }
}
