using System;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CreatorDK.IO.DPipes
{
    public enum DP_MESSAGE_TYPE
    {
        MESSAGE = 1,
        MESSAGE_INFO = 2,
        MESSAGE_WARNING = 3,
        MESSAGE_ERROR = 4
    }

    public class DPMessangerBase
    {
        protected readonly DPipe _dpipe;
        protected Encoding _encoding;
        protected Mutex _mutexWritePipe;

        public delegate void MessageStringRecevied(string message);
        public delegate void MessageDataRecevied(byte[] data);
        public Encoding Encoding 
        { 
            get 
            {
                if (_encoding == null)
                    _encoding = Encoding.Unicode;

                return _encoding; 
            }

            set
            {
                if (value == null)
                    _encoding = Encoding.Unicode;
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
        public DPMessangerBase(DPipe dpipe, Encoding encoding, uint stringBufferSize = 4096)
        {
            _dpipe = dpipe;
            Encoding = encoding;
            _mutexWritePipe = new Mutex(false);
        }
        public static Encoding GetEncoding(uint encodingCode)
        {
            switch (encodingCode)
            {
                case 1: 
                    return Encoding.Unicode;
                default:
                    return Encoding.UTF8;
            }
        }
        public static uint GetEncodingCode(PacketHeader header)
        {
            if (header.IsService)
                return header.ServicePrefix;

            return header.DataPrefix;
        }
        public static uint GetEncodingCode(Encoding encoding)
        {
            if (encoding == null)
                return 1;

            if (encoding == Encoding.UTF8)
                return 0;

            return 1;
        }
        public Encoding GetEncoding(PacketHeader header)
        {
            var encodingCode = GetEncodingCode(header);
            return GetEncoding(encodingCode);
        }
        public static uint AddEncodingCode(uint serviceCode, uint encodingCode)
        {
            var encodingRaw = encodingCode << 24;
            return serviceCode | encodingRaw;
        }
        public virtual string GetString(PacketHeader header, Encoding encoding = null)
        {
            var buffer = new byte[header.DataSize];

            _dpipe.Read(buffer, 0, header.DataSize);

            if (encoding == null)
            {
                var encodingCode = GetEncodingCode(header);
                encoding = GetEncoding(encodingCode);
            }

            return encoding.GetString(buffer, 0, header.DataSize);
        }

        public string GetString(PacketHeader header, byte[] data)
        {
            return GetEncoding(header).GetString(data);
        }
        protected virtual void OnMessageDataReceivedInner(PacketHeader header, byte[] data)
        {
            //Get data code ignoring first 8 bits (using for encoding)
            switch (header.DataCodeOnly) 
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
            //Removing Encoding code from ServiceCode
            switch (header.DataCodeOnly)
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
        public virtual void SendMessage(string message, Encoding encoding = null)
        {
            var encodingCurrent = encoding == null ? Encoding : encoding;

            byte[] data = encodingCurrent.GetBytes(message);

            var code = AddEncodingCode(Constants.DP_MESSAGE_STRING, GetEncodingCode(encodingCurrent));

            _mutexWritePipe.WaitOne();
                _dpipe.Write(code, data, 0, data.Length);
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
        public virtual void SendInfo(string message, Encoding encoding = null)
        {
            var encodingCurrent = encoding == null ? Encoding : encoding;

            byte[] data = encodingCurrent.GetBytes(message);

            var code = AddEncodingCode(Constants.DP_INFO_STRING, GetEncodingCode(encodingCurrent));

            _mutexWritePipe.WaitOne();
                _dpipe.Write(code, data, 0, data.Length);
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
        public virtual void SendWarning(string message, Encoding encoding = null)
        {
            var encodingCurrent = encoding == null ? Encoding : encoding;

            byte[] data = encodingCurrent.GetBytes(message);

            var code = AddEncodingCode(Constants.DP_WARNING_STRING, GetEncodingCode(encodingCurrent));

            _mutexWritePipe.WaitOne();
                _dpipe.Write(code, data, 0, data.Length);
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
        public virtual void SendError(string message, Encoding encoding = null)
        {
            var encodingCurrent = encoding == null ? Encoding : encoding;

            byte[] data = encodingCurrent.GetBytes(message);

            var code = AddEncodingCode(Constants.DP_WARNING_STRING, GetEncodingCode(encodingCurrent));

            _mutexWritePipe.WaitOne();
                _dpipe.Write(code, data, 0, data.Length);
            _mutexWritePipe.ReleaseMutex();
        }
        public virtual void SendErrorAsync(string message)
        {
            Task.Run(() => SendError(message));
        }

        public void Send(DP_MESSAGE_TYPE messageType, byte[] data)
        {
            switch(messageType) 
            { 
                case DP_MESSAGE_TYPE.MESSAGE:
                    SendMessage(data); break;
                case DP_MESSAGE_TYPE.MESSAGE_INFO:
                    SendInfo(data); break;
                case DP_MESSAGE_TYPE.MESSAGE_WARNING:
                    SendWarning(data); break;
                case DP_MESSAGE_TYPE.MESSAGE_ERROR:
                    SendError(data); break;
                default:
                    throw new Exception("Unknown message type");
            }
        }
        public void SendAsync(DP_MESSAGE_TYPE messageType, byte[] data)
        {
            Task.Run(() => Send(messageType, data));
        }

        public void Send(DP_MESSAGE_TYPE messageType, string message, Encoding encoding = null)
        {
            switch (messageType)
            {
                case DP_MESSAGE_TYPE.MESSAGE:
                    SendMessage(message, encoding); 
                    break;
                case DP_MESSAGE_TYPE.MESSAGE_INFO:
                    SendInfo(message, encoding); 
                    break;
                case DP_MESSAGE_TYPE.MESSAGE_WARNING:
                    SendWarning(message, encoding); 
                    break;
                case DP_MESSAGE_TYPE.MESSAGE_ERROR:
                    SendError(message, encoding); 
                    break;
                default:
                    throw new Exception("Unknown message type");
            }
        }
        public void SendAsync(DP_MESSAGE_TYPE messageType, string message, Encoding encoding = null)
        {
            Task.Run(() => Send(messageType, message, encoding));
        }

        public void Disconnect()
        {
            _dpipe.Disconnect();
        }
        public virtual void Disconnect(string disconnectMessage, Encoding encoding = null)
        {
            if (disconnectMessage == null)
                Disconnect();

            encoding = encoding == null ? Encoding : encoding;

            byte[] disonnectData = null;

            if (!string.IsNullOrEmpty(disconnectMessage))
                disonnectData = encoding.GetBytes(disconnectMessage);

            var encodingCode = GetEncodingCode(encoding);

            _dpipe.Disconnect(disonnectData, encodingCode);
        }
    }
}
