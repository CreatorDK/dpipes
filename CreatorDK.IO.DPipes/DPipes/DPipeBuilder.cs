using System.Xml.Linq;

namespace CreatorDK.IO.DPipes
{
    public static class DPipeBuilder
    {
        public static DPipe Create(DP_TYPE pipeType, int inBufferSize, int outBufferSize)
        {
            switch (pipeType)
            {
                case DP_TYPE.ANONYMOUS_PIPE:
                    return new DPAnonymous(inBufferSize, outBufferSize);
                case DP_TYPE.NAMED_PIPE:
                    return new DPNamed(inBufferSize, outBufferSize);
                default:
                    return null;
            }
        }
        public static DPipe Create(DP_TYPE pipeType, string name)
        {
            switch (pipeType)
            {
                case DP_TYPE.ANONYMOUS_PIPE:
                    return new DPAnonymous(name);
                case DP_TYPE.NAMED_PIPE:
                    return DPNamed.Create(name);
                default:
                    return null;
            }
        }
        public static DPipe Create(DP_TYPE pipeType, string name, int inBufferSize, int outBufferSize)
        {
            switch (pipeType)
            {
                case DP_TYPE.ANONYMOUS_PIPE:
                    return new DPAnonymous(name, inBufferSize, outBufferSize);
                case DP_TYPE.NAMED_PIPE:
                    return DPNamed.Create(name, inBufferSize, outBufferSize);
                default:
                    return null;
            }

        }
        public static DPipe Create(DP_TYPE pipeType)
        {
            switch (pipeType)
            {
                case DP_TYPE.ANONYMOUS_PIPE:
                    return new DPAnonymous();
                case DP_TYPE.NAMED_PIPE:
                    return new DPNamed();
                default:
                    return null;
            }
        }
        public static DPipe Create(string pipeHandle, int inBufferSize, int outBufferSize, bool connect = false)
        {
            if (DPAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymousPipe = new DPAnonymous(inBufferSize, outBufferSize);
                if (connect)
                    anonymousPipe.Connect(pipeHandle);
                return anonymousPipe;
            }

            else if (DPNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = new DPNamed(inBufferSize, outBufferSize);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe Create(string pipeHandle, string name, int inBufferSize, int outBufferSize, bool connect = false)
        {
            if (DPAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymousPipe = new DPAnonymous(name, inBufferSize, outBufferSize);
                if (connect)
                    anonymousPipe.Connect(pipeHandle);
                return anonymousPipe;
            }

            else if (DPNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = DPNamed.Create(name, inBufferSize, outBufferSize);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe Create(string pipeHandle, string name, bool connect = false)
        {
            if (DPAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymousPipe = new DPAnonymous(name);
                if (connect)
                    anonymousPipe.Connect(pipeHandle);
                return anonymousPipe;
            }

            else if (DPNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = DPNamed.Create(name);
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
        public static DPipe Create(string pipeHandle, bool connect = false)
        {
            if (DPAnonymousHandle.IsAnonymus(pipeHandle))
            {
                var anonymousPipe = new DPAnonymous();
                if (connect)
                    anonymousPipe.Connect(pipeHandle);
                return anonymousPipe;
            }

            else if (DPNamedHandle.IsNamed(pipeHandle))
            {
                var namedPipe = new DPNamed();
                if (connect)
                    namedPipe.Connect(pipeHandle);
                return namedPipe;
            }
            else
                return null;
        }
    }
}
