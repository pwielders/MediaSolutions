namespace TraceClient
{
    partial class TraceWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.portNumber = new System.Windows.Forms.NumericUpDown();
            this.Listen = new System.Windows.Forms.Button();
            this.logWindow = new System.Windows.Forms.DataGridView();
            this.timeStamp = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.className = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.lineNumber = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.FileName = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.message = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.config = new System.Windows.Forms.Button();
            this.hostName = new System.Windows.Forms.TextBox();
            this.btnClear = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.portNumber)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.logWindow)).BeginInit();
            this.SuspendLayout();
            // 
            // portNumber
            // 
            this.portNumber.Location = new System.Drawing.Point(359, 618);
            this.portNumber.Maximum = new decimal(new int[] {
            65535,
            0,
            0,
            0});
            this.portNumber.Minimum = new decimal(new int[] {
            1024,
            0,
            0,
            0});
            this.portNumber.Name = "portNumber";
            this.portNumber.Size = new System.Drawing.Size(56, 20);
            this.portNumber.TabIndex = 0;
            this.portNumber.Value = new decimal(new int[] {
            5555,
            0,
            0,
            0});
            // 
            // Listen
            // 
            this.Listen.Location = new System.Drawing.Point(12, 615);
            this.Listen.Name = "Listen";
            this.Listen.Size = new System.Drawing.Size(75, 23);
            this.Listen.TabIndex = 3;
            this.Listen.Text = "Listen";
            this.Listen.UseVisualStyleBackColor = true;
            this.Listen.Click += new System.EventHandler(this.Listen_Click);
            // 
            // logWindow
            // 
            this.logWindow.AllowUserToAddRows = false;
            this.logWindow.AllowUserToDeleteRows = false;
            this.logWindow.AllowUserToOrderColumns = true;
            this.logWindow.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.logWindow.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.timeStamp,
            this.className,
            this.lineNumber,
            this.FileName,
            this.message});
            this.logWindow.Location = new System.Drawing.Point(12, 12);
            this.logWindow.Name = "logWindow";
            this.logWindow.ReadOnly = true;
            this.logWindow.Size = new System.Drawing.Size(1063, 597);
            this.logWindow.TabIndex = 4;
            // 
            // timeStamp
            // 
            this.timeStamp.HeaderText = "TimeStamp";
            this.timeStamp.Name = "timeStamp";
            this.timeStamp.ReadOnly = true;
            // 
            // className
            // 
            this.className.HeaderText = "ClassName";
            this.className.Name = "className";
            this.className.ReadOnly = true;
            this.className.Width = 120;
            // 
            // lineNumber
            // 
            this.lineNumber.HeaderText = "Line";
            this.lineNumber.Name = "lineNumber";
            this.lineNumber.ReadOnly = true;
            this.lineNumber.Width = 80;
            // 
            // FileName
            // 
            this.FileName.HeaderText = "File";
            this.FileName.Name = "FileName";
            this.FileName.ReadOnly = true;
            this.FileName.Width = 120;
            // 
            // message
            // 
            this.message.HeaderText = "Message";
            this.message.Name = "message";
            this.message.ReadOnly = true;
            this.message.Width = 600;
            // 
            // config
            // 
            this.config.Location = new System.Drawing.Point(988, 615);
            this.config.Name = "config";
            this.config.Size = new System.Drawing.Size(87, 23);
            this.config.TabIndex = 5;
            this.config.Text = "Configuration";
            this.config.UseVisualStyleBackColor = true;
            this.config.Click += new System.EventHandler(this.config_Click);
            // 
            // hostName
            // 
            this.hostName.Location = new System.Drawing.Point(126, 617);
            this.hostName.Name = "hostName";
            this.hostName.Size = new System.Drawing.Size(227, 20);
            this.hostName.TabIndex = 6;
            this.hostName.Text = "localhost";
            // 
            // btnClear
            // 
            this.btnClear.Location = new System.Drawing.Point(895, 614);
            this.btnClear.Name = "btnClear";
            this.btnClear.Size = new System.Drawing.Size(87, 23);
            this.btnClear.TabIndex = 7;
            this.btnClear.Text = "Clear";
            this.btnClear.UseVisualStyleBackColor = true;
            this.btnClear.Click += new System.EventHandler(this.btnClear_Click);
            // 
            // TraceWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1087, 644);
            this.Controls.Add(this.btnClear);
            this.Controls.Add(this.hostName);
            this.Controls.Add(this.config);
            this.Controls.Add(this.logWindow);
            this.Controls.Add(this.Listen);
            this.Controls.Add(this.portNumber);
            this.Name = "TraceWindow";
            this.Text = "Trace window";
            ((System.ComponentModel.ISupportInitialize)(this.portNumber)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.logWindow)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.NumericUpDown portNumber;
        private System.Windows.Forms.Button Listen;
        private System.Windows.Forms.DataGridView logWindow;
        private System.Windows.Forms.DataGridViewTextBoxColumn timeStamp;
        private System.Windows.Forms.DataGridViewTextBoxColumn className;
        private System.Windows.Forms.DataGridViewTextBoxColumn lineNumber;
        private System.Windows.Forms.DataGridViewTextBoxColumn FileName;
        private System.Windows.Forms.DataGridViewTextBoxColumn message;
        private System.Windows.Forms.Button config;
        private System.Windows.Forms.TextBox hostName;
        private System.Windows.Forms.Button btnClear;
    }
}

