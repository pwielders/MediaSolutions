using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Net;

namespace TraceClient
{
    public partial class TraceWindow : Form
    {
        public TraceWindow()
        {
            InitializeComponent();
            TraceMedia.Instance.RequestTraceLInes(new TraceMedia.ReceiveTraceLine(SetText));
        }

        public void SetText(TraceLine info)
        {
            if (InvokeRequired)
            {
                Invoke(new TraceMedia.ReceiveTraceLine(SetText), new object[] { info });
            }
            else
            {
                DataGridViewRow addedRow = logWindow.Rows[logWindow.Rows.Add()];

                addedRow.Cells[0].Value = info.Time.ToString("HH:mm.ss.fff");
                addedRow.Cells[1].Value = info.ClassName;
                addedRow.Cells[2].Value = info.LineNumber.ToString();
                addedRow.Cells[3].Value = System.IO.Path.GetFileName(info.FileName);
                addedRow.Cells[4].Value = info.Message;
            }
        }

        private void Listen_Click(object sender, EventArgs e)
        {
            if (TraceMedia.Instance.IsActive)
            {
                Listen.Text = "Listen";
                TraceMedia.Instance.Deactivate();
            }
            else
            {
                Listen.Text = "Idle";
                TraceMedia.Instance.Activate(hostName.Text, int.Parse(portNumber.Value.ToString()));
            }
        }

        private void config_Click(object sender, EventArgs e)
        {
            new TraceConfig().ShowDialog();
        }

        private void btnClear_Click(object sender, EventArgs e)
        {
            logWindow.Rows.Clear();
        }
    }
}
