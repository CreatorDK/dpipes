using System.Xml.Linq;

namespace CreatorDK.IO.DPipes
{
    public static class DPipeBuilder
    {
        public static DPipe? Create(DPIPE_TYPE pipeType, int inBufferSize, int outBufferSize)
        {
            return pipeType switch
            {
                DPIPE_TYPE.ANONYMUS_PIPE => new DPipeAnonymus(inBufferSize, outBufferSize),
                DPIPE_TYPE.NAMED_PIPE => new DPipeNamed(inBufferSize, outBufferSize),
                _ => null,
            };
        }
        public static DPipe? Create(DPIPE_TYPE pipeType, string name)
        {
            return pipeType switch
            {
                DPIPE_TYPE.ANONYMUS_PIPE => new DPipeAnonymus(name),
                DPIPE_TYPE.NAMED_PIPE => DPipeNamed.Create(name),
                _ => null,
            };
        }
        public static DPipe? Create(DPIPE_TYPE pipeType, string name, int inBufferSize, int outBufferSize)
        {
            return pipeType switch
            {
                DPIPE_TYPE.ANONYMUS_PIPE => new DPipeAnonymus(name, inBufferSize, outBufferSize),
                DPIPE_TYPE.NAMED_PIPE => DPipeNamed.Create(name, inBufferSize, outBufferSize),
                _ => null,
            };
        }
        public static DPipe? Create(DPIPE_TYPE pipeType)
        {
            return pipeType switch
            {
                DPIPE_TYPE.ANONYMUS_PIPE => new DPipeAnonymus(),
                DPIPE_TYPE.NAMED_PIPE => new DPipeNamed(),
                _ => null,
            };
        }
        public static DPipe? Create(string pipeHandle, int inBufferSize, int outBufferSize, bool connect = false)
        {
            if (DPipeAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymusPipe = new DPipeAnonymus(inBufferSize, outBufferSize);
                if (connect)
                    anonymusPipe.Connect(pipeHandle);
                return anonymusPipe;
            }

            else if (DPipeNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = new DPipeNamed(inBufferSize, outBufferSize);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe? Create(string pipeHandle, string name, int inBufferSize, int outBufferSize, bool connect = false)
        {
            if (DPipeAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymusPipe = new DPipeAnonymus(name, inBufferSize, outBufferSize);
                if (connect)
                    anonymusPipe.Connect(pipeHandle);
                return anonymusPipe;
            }

            else if (DPipeNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = DPipeNamed.Create(name, inBufferSize, outBufferSize);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe? Create(string pipeHandle, string name, bool connect = false)
        {
            if (DPipeAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymusPipe = new DPipeAnonymus(name);
                if (connect)
                    anonymusPipe.Connect(pipeHandle);
                return anonymusPipe;
            }

            else if (DPipeNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = DPipeNamed.Create(name);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe? Create(string pipeHandle, bool connect = false)
        {
            if (DPipeAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymusPipe = new DPipeAnonymus();
                if (connect)
                    anonymusPipe.Connect(pipeHandle);
                return anonymusPipe;
            }

            else if (DPipeNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = new DPipeNamed();
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
    }
}
