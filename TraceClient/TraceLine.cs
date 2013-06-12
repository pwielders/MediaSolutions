using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace TraceClient
{
    public class TraceLine
    {
        public TraceLine(byte[] message)
        {
            IsTraceLine = (message[0] == 'T');
            IsResponseLine = (message[0] == 'R');

            if (IsTraceLine)
            {
                long dateTime = ((long)message[1] << 56) | ((long)message[2] << 48) | ((long)message[3] << 40) | ((long)message[4] << 32) | ((long)message[5] << 24) | ((long)message[6] << 16) | ((long)message[7] << 8) | ((long)message[8] << 0); 
                LineNumber = (message[9] << 24) | (message[10] << 16) | (message[11] << 8) | (message[12] << 0);;

                Time = new DateTime(dateTime, DateTimeKind.Utc);

                String[] info = Encoding.ASCII.GetString(message, 13, message.Length - 13).Split('\0');

                FileName = info[0];
                ClassName = info[1];
                Message = info[2];
            }
            if (IsResponseLine)
            {
                SequenceNumber = (message[1] << 24) | (message[2] << 16) | (message[3] << 8) | (message[4] << 0);
                Command = message[5];

                if (Command == 0)
                {
                    Active = (message[6] == '1');
                    String[] info = Encoding.ASCII.GetString(message, 8, message.Length - 8).Split('\0');

                    Category = info[0];
                    Module = info[1];
                }
                if (Command == 1)
                {
                    Updates = (message[6] << 24) | (message[7] << 16) | (message[8] << 8) | (message[9] << 0);
                }
            }
        }

        public bool IsResponseLine { get; private set; }
        public bool IsTraceLine { get; private set; }
        public String Message { get; private set; }
        public String ClassName { get; private set; }
        public String FileName { get; private set; }
        public String Module { get; private set; }
        public String Category { get; private set; }
        public bool Active { get; private set; }
        public int LineNumber { get; private set; }
        public DateTime Time { get; private set; }
        public int SequenceNumber { get; private set; }
        public int Command { get; private set; }
        public int Updates { get; private set; }
    }
}
