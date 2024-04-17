using System;

namespace CreatorDK.IO.DPipes
{
    public struct DPResponseHeader
    {
        public Guid Guid {  get; set; }
        public int Code {  get; set; }
        public int DataType { get; set; }
        public int DescriptorType { get; set; }
        public int DescriptorSize { get; set; }
    }
}
