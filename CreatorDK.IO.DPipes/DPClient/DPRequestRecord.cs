using System;

namespace CreatorDK.IO.DPipes
{
    public class DPRequestRecord
    {
        public Guid Guid {  get; set; }
        public bool IsSucess { get; set; }
        public DPResponse Response { get; set; }
    }
}
