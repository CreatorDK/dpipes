namespace CreatorDK.IO.DPipes
{
    public struct DPRequestHeader
    {
        public Guid Guid {  get; set; }
        public int Code {  get; set; }
        public int DataType { get; set; }
    }
}
