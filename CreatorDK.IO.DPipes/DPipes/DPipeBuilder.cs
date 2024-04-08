using System.Xml.Linq;

namespace CreatorDK.IO.DPipes
{
    public static class DPipeBuilder
    {
        public static DPipe Create(DPIPE_TYPE pipeType, int inBufferSize, int outBufferSize)
        {
            switch (pipeType)
            {
                case DPIPE_TYPE.ANONYMUS_PIPE:
                    return new DPipeAnonymus(inBufferSize, outBufferSize);
                case DPIPE_TYPE.NAMED_PIPE:
                    return new DPipeNamed(inBufferSize, outBufferSize);
                default:
                    return null;
            }
        }
        public static DPipe Create(DPIPE_TYPE pipeType, string name)
        {
            switch (pipeType)
            {
                case DPIPE_TYPE.ANONYMUS_PIPE:
                    return new DPipeAnonymus(name);
                case DPIPE_TYPE.NAMED_PIPE:
                    return DPipeNamed.Create(name);
                default:
                    return null;
            }
        }
        public static DPipe Create(DPIPE_TYPE pipeType, string name, int inBufferSize, int outBufferSize)
        {
            switch (pipeType)
            {
                case DPIPE_TYPE.ANONYMUS_PIPE:
                    return new DPipeAnonymus(name, inBufferSize, outBufferSize);
                case DPIPE_TYPE.NAMED_PIPE:
                    return DPipeNamed.Create(name, inBufferSize, outBufferSize);
                default:
                    return null;
            }

        }
        public static DPipe Create(DPIPE_TYPE pipeType)
        {
            switch (pipeType)
            {
                case DPIPE_TYPE.ANONYMUS_PIPE:
                    return new DPipeAnonymus();
                case DPIPE_TYPE.NAMED_PIPE:
                    return new DPipeNamed();
                default:
                    return null;
            }
        }
        public static DPipe Create(string pipeHandle, int inBufferSize, int outBufferSize, bool connect = false)
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
        public static DPipe Create(string pipeHandle, string name, int inBufferSize, int outBufferSize, bool connect = false)
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
        public static DPipe Create(string pipeHandle, string name, bool connect = false)
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
        public static DPipe Create(string pipeHandle, bool connect = false)
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
