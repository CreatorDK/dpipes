namespace CreatorDK.IO.DPipes
{
    public interface IDPipeHandle
    {
        DPIPE_TYPE GetType();
        string AsString();
    }
}
