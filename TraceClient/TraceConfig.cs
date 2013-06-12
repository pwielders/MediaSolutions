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
    public partial class TraceConfig : Form
    {
        private bool m_Updating = false;

        public TraceConfig()
        {
            InitializeComponent();
        }

        private void TraceConfig_Load(object sender, EventArgs e)
        {
            UpdateConfiguration();
        }

        private void UpdateConfiguration()
        {
            if (TraceMedia.Instance.IsActive)
            {
                TraceMedia.Instance.RequestOverview(new TraceMedia.ReceiveRespone(ResponseEntry));
            }
        }

        public void ResponseEntry(TraceLine info)
        {
            if (InvokeRequired)
            {
                Invoke(new TraceMedia.ReceiveRespone(ResponseEntry), new object[] { info });
            }
            else if (info.Command == 0)
            {
                m_Updating = true;

                if (onModule.Checked)
                {
                    TreeNode attach = traceTreeView.Nodes[info.Module];

                    if (attach == null)
                    {
                        attach = traceTreeView.Nodes.Add(info.Module);
                        attach.Name = info.Module;
                    }

                    TreeNode item = attach.Nodes[info.Category];

                    if (item == null)
                    {
                        // Add this category to this node
                        item = attach.Nodes.Add(info.Category);
                        item.Name = info.Category;
                    }

                    item.Checked = info.Active;
                }
                else
                {
                    TreeNode attach = traceTreeView.Nodes[info.Category];

                    if (attach == null)
                    {
                        attach = traceTreeView.Nodes.Add(info.Category);
                        attach.Name = info.Category;
                    }

                    TreeNode item = attach.Nodes[info.Module];

                    if (item == null)
                    {
                        // Add this category to this node
                        item = attach.Nodes.Add(info.Module);
                        item.Name = info.Module;
                    }

                    item.Checked = info.Active;
                }

                m_Updating = false;
            }
            else if (info.Command == 1)
            {
                TraceMedia.Instance.RequestOverview(new TraceMedia.ReceiveRespone(ResponseEntry));
            }
        }

        private void traceTreeView_AfterCheck(object sender, TreeViewEventArgs e)
        {
            if (m_Updating == false)
            {
                String module = String.Empty;
                String category = String.Empty;

                if (onModule.Checked)
                {
                    if (e.Node.Parent == null)
                    {
                        module = e.Node.Name;
                    }
                    else
                    {
                        module = e.Node.Parent.Name;
                        category = e.Node.Name;
                    }
                }
                else
                {
                    if (e.Node.Parent == null)
                    {
                        category = e.Node.Name;
                    }
                    else
                    {
                        module = e.Node.Name;
                        category = e.Node.Parent.Name;
                    }
                }

                TraceMedia.Instance.SetTraceLevel(module, category, e.Node.Checked, new TraceMedia.ReceiveRespone(ResponseEntry));
            }
        }

        private void onModule_CheckedChanged(object sender, EventArgs e)
        {
            traceTreeView.Nodes.Clear();
            UpdateConfiguration();
        }

        private void onCategory_CheckedChanged(object sender, EventArgs e)
        {
            traceTreeView.Nodes.Clear();
            UpdateConfiguration();
        }

        private void reload_Click(object sender, EventArgs e)
        {
            UpdateConfiguration();
        }
    }
}
