using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;

namespace TraceClient
{
    public class TraceMedia
    {
        private uint m_Sequence = 0;
        private bool m_Running;
        private UdpClient m_Reader;
        private System.Threading.Thread m_Worker;
        private System.Net.IPEndPoint m_RemoteNode;
        private ReceiveTraceLine m_TraceLineCallback;
        private ReceiveRespone m_ResponseCallback;

        public delegate void ReceiveTraceLine(TraceLine info);
        public delegate void ReceiveRespone(TraceLine info);

        public void Activate(String hostName, int portNumber)
        {
            if (m_Running)
            {
                Deactivate();
            }

            System.Net.IPAddress address = System.Net.IPAddress.Loopback;

            foreach (var element in System.Net.Dns.GetHostAddresses(hostName))
            {
                if (element.AddressFamily == AddressFamily.InterNetwork)
                {
                    address = element;
                    break;
                }
            }

            m_RemoteNode = new System.Net.IPEndPoint(address, portNumber + 1);
            m_Reader = new UdpClient(new System.Net.IPEndPoint(address, portNumber));
            m_Running = true;
            m_Worker = new System.Threading.Thread(new System.Threading.ThreadStart(worker));
            m_Worker.Start();
        }

        public void Deactivate()
        {
            if (m_Running)
            {
                m_Running = false;
                m_Reader.Close();
                m_RemoteNode = null;
                m_Reader = null;
            }
        }

        private void worker()
        {
            System.Net.IPEndPoint remoteNode = new System.Net.IPEndPoint (m_RemoteNode.Address, m_RemoteNode.Port);

            while (m_Running == true)
            {
                try
                {
                    byte[] stream = m_Reader.Receive(ref remoteNode);

                    TraceLine newInfo = new TraceLine(stream);

                    if ((newInfo.IsTraceLine)  && (m_TraceLineCallback != null))
                    {
                        // It's on a different thread, so use Invoke.
                        m_TraceLineCallback (newInfo);
                    }
                    else if ((newInfo.IsResponseLine) && (m_ResponseCallback != null) && (newInfo.SequenceNumber == m_Sequence))
                    {
                        // It's on a different thread, so use Invoke.
                        m_ResponseCallback ( newInfo );
                    }
                }
                catch(Exception)
                {
                }
            }            
        }

        public void RequestTraceLInes(ReceiveTraceLine callback)
        {
            m_TraceLineCallback = callback;
        }

        public void RequestOverview(ReceiveRespone callback)
        {
            m_ResponseCallback = callback;

            byte[] message = new byte[10];

            m_Sequence++;

            message[0] = (byte)'C';
            message[1] = (byte)((m_Sequence >> 24) & 0xFF);
            message[2] = (byte)((m_Sequence >> 16) & 0xFF);
            message[3] = (byte)((m_Sequence >> 8) & 0xFF);
            message[4] = (byte)((m_Sequence >> 0) & 0xFF);
            message[5] = 0;       // Request a list of categories/Modules..

            m_Reader.Send(message, 6, m_RemoteNode);
        }

        public void SetTraceLevel(String module, String category, bool enabled, ReceiveRespone callback)
        {
            m_ResponseCallback = callback;

            byte[] message = new byte[256];

            m_Sequence++;

            message[0] = (byte)'C';
            message[1] = (byte)((m_Sequence >> 24) & 0xFF);
            message[2] = (byte)((m_Sequence >> 16) & 0xFF);
            message[3] = (byte)((m_Sequence >> 8) & 0xFF);
            message[4] = (byte)((m_Sequence >> 0) & 0xFF);
            message[5] = 1;       // Togggle Setting of TraceCategory
            message[6] = (byte)(enabled ? '1' : '0');
            message[7] = (byte) '\0';

            int offset = 8;

            offset = CopyText(message, offset, category);
            offset = CopyText(message, offset, module);

            m_Reader.Send(message, offset+1, m_RemoteNode);
        }

        public bool IsActive { get { return (m_Reader != null); } }

        public static TraceMedia Instance { get { return (m_TraceMedia); } }

        private int CopyText(byte[] buffer, int offset, String value)
        {
            int index = 0;
            while ((offset < buffer.Length) && (index < value.Length))
            {
                buffer[offset] = (byte)value[index];
                index++;
                offset++;
            }
            buffer[offset] = (byte)'\0';
            offset++;

            return (offset);
        }

        private static TraceMedia m_TraceMedia = new TraceMedia();
    }
}
