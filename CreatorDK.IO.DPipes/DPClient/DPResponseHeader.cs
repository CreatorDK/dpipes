namespace CreatorDK.IO.DPipes
{
    public struct DPResponseHeader
    {
        public Guid Guid {  get; set; }
        public int Code {  get; set; }
        public int DataType { get; set; }
    }
}
