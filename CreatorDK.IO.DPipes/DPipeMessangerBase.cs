using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public enum DPipeMessageType
    {
        Message = 1,
        Info = 2,
        Warning = 3,
        Error = 4
    }

    public class DPipeMessangerBase
    {
        protected readonly DPipe _dpipe;
        protected Encoding _encoding;
        protected Mutex _mutexWritePipe;
        protected byte[] _stringBuffer;

        public delegate void MessageStringRecevied(string message);
        public delegate void MessageDataRecevied(byte[] data);
        public Encoding Encoding 
        { 
            get 
            {
                return _encoding; 
            }

            set
            {
                if (value == null)
                    _encoding = Encoding.Default;
                else
                    _encoding = value;
            }
        }
        public MessageStringRecevied OnMessageStringReceived { get; set; }
        public MessageDataRecevied OnMessageDataReceived { get; set; }
        public MessageStringRecevied OnInfoStringReceived { get; set; }
        public MessageDataRecevied OnInfoDataReceived { get; set; }
        public MessageStringRecevied OnWarningStringReceived { get; set; }
        public MessageDataRecevied OnWarningDataReceived { get; set; }
        public MessageStringRecevied OnErrorStringReceived { get; set; }
        public MessageDataRecevied OnErrorDataReceived { get; set; }
        public DPipeMessangerBase(DPipe dpipe, Encoding encoding, uint stringBufferSize = 4096)
        {
            _dpipe = dpipe;
            _encoding = encoding == null ? Encoding.Default : encoding;
            _mutexWritePipe = new Mutex(false);
            _stringBuffer = new byte[stringBufferSize];
        }
        protected virtual void CheckStringBufferSize(int size)
        {
            if (size > _stringBuffer.Length) 
                _stringBuffer = new byte[(int)(size * 1.5)];
        }
        protected virtual string GetStringFromPipe(int size)
        {
            CheckStringBufferSize(size);
            _dpipe.Read(_stringBuffer, 0, size);
            return _encoding.GetString(_stringBuffer, 0, size);
        }
        protected virtual void OnMessageDataReceivedInner(PacketHeader header, byte[] data)
        {
            switch (header.Command) 
            {
                case Constants.DP_MESSAGE_DATA:
                    OnMessageDataReceived?.Invoke(data);
                    break;
                case Constants.DP_INFO_DATA:
                    OnInfoDataReceived?.Invoke(data);
                    break;
                case Constants.DP_WARNING_DATA:
                    OnWarningDataReceived?.Invoke(data);
                    break;
                case Constants.DP_ERROR_DATA:
                    OnErrorDataReceived?.Invoke(data);
                    break;
            }
        }
        protected virtual void OnMessageStringReceivedInner(PacketHeader header, string message)
        {
            switch (header.Command)
            {
                case Constants.DP_MESSAGE_STRING:
                    OnMessageStringReceived?.Invoke(message);
                    break;
                case Constants.DP_INFO_STRING:
                    OnInfoStringReceived?.Invoke(message);
                    break;
                case Constants.DP_WARNING_STRING:
                    OnWarningStringReceived?.Invoke(message);
                    break;
                case Constants.DP_ERROR_STRING:
                    OnErrorStringReceived?.Invoke(message);
                    break;
            }
        }
        public virtual void SendMessage(byte[] data)
        {
            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_MESSAGE_DATA, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendMessageAsync(byte[] data) 
        {
            Task.Run(() => SendMessage(data));
        }
        public virtual void SendMessage(string message)
        {
            byte[] data = _encoding.GetBytes(message);

            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_MESSAGE_STRING, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendMessageAsync(string message)
        {
            Task.Run(() => SendMessage(message));
        }
        public virtual void SendInfo(byte[] data)
        {
            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_INFO_DATA, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendInfoAsync(byte[] data)
        {
            Task.Run(() => SendInfo(data));
        }
        public virtual void SendInfo(string message)
        {
            byte[] data = _encoding.GetBytes(message);

            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_INFO_STRING, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendInfoAsync(string message)
        {
            Task.Run(() => SendInfo(message));
        }
        public virtual void SendWarning(byte[] data)
        {
            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_WARNING_DATA, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendWarningAsync(byte[] data)
        {
            Task.Run(() => SendWarning(data));
        }
        public virtual void SendWarning(string message)
        {
            byte[] data = _encoding.GetBytes(message);

            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_WARNING_STRING, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendWarningAsync(string message)
        {
            Task.Run(() => SendWarning(message));
        }
        public virtual void SendError(byte[] data)
        {
            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_ERROR_DATA, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendErrorAsync(byte[] data)
        {
            Task.Run(() => SendError(data));
        }
        public virtual void SendError(string message)
        {
            byte[] data = _encoding.GetBytes(message);

            _mutexWritePipe.WaitOne();
                _dpipe.Write(Constants.DP_ERROR_STRING, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendErrorAsync(string message)
        {
            Task.Run(() => SendError(message));
        }

        public void Send(DPipeMessageType messageType, byte[] data)
        {
            switch(messageType) 
            { 
                case DPipeMessageType.Message:
                    SendMessage(data); break;
                case DPipeMessageType.Info:
                    SendInfo(data); break;
                case DPipeMessageType.Warning:
                    SendWarning(data); break;
                case DPipeMessageType.Error:
                    SendError(data); break;
                default:
                    throw new Exception("Unknown message type");
            }
        }
        public void SendAsync(DPipeMessageType messageType, byte[] data)
        {
            Task.Run(() => Send(messageType, data));
        }

        public void Send(DPipeMessageType messageType, string message)
        {
            switch (messageType)
            {
                case DPipeMessageType.Message:
                    SendMessage(message); break;
                case DPipeMessageType.Info:
                    SendInfo(message); break;
                case DPipeMessageType.Warning:
                    SendWarning(message); break;
                case DPipeMessageType.Error:
                    SendError(message); break;
                default:
                    throw new Exception("Unknown message type");
            }
        }
        public void SendAsync(DPipeMessageType messageType, string message)
        {
            Task.Run(() => Send(messageType, message));
        }
    }
}
