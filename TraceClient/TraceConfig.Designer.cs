namespace TraceClient
{
    partial class TraceConfig
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
            this.traceTreeView = new System.Windows.Forms.TreeView();
            this.SortOrder = new System.Windows.Forms.GroupBox();
            this.onCategory = new System.Windows.Forms.RadioButton();
            this.onModule = new System.Windows.Forms.RadioButton();
            this.reload = new System.Windows.Forms.Button();
            this.SortOrder.SuspendLayout();
            this.SuspendLayout();
            // 
            // traceTreeView
            // 
            this.traceTreeView.CheckBoxes = true;
            this.traceTreeView.Location = new System.Drawing.Point(12, 12);
            this.traceTreeView.Name = "traceTreeView";
            this.traceTreeView.Size = new System.Drawing.Size(338, 311);
            this.traceTreeView.TabIndex = 0;
            this.traceTreeView.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.traceTreeView_AfterCheck);
            // 
            // SortOrder
            // 
            this.SortOrder.Controls.Add(this.onCategory);
            this.SortOrder.Controls.Add(this.onModule);
            this.SortOrder.Location = new System.Drawing.Point(12, 329);
            this.SortOrder.Name = "SortOrder";
            this.SortOrder.Size = new System.Drawing.Size(245, 37);
            this.SortOrder.TabIndex = 1;
            this.SortOrder.TabStop = false;
            // 
            // onCategory
            // 
            this.onCategory.AutoSize = true;
            this.onCategory.Location = new System.Drawing.Point(133, 14);
            this.onCategory.Name = "onCategory";
            this.onCategory.Size = new System.Drawing.Size(104, 17);
            this.onCategory.TabIndex = 1;
            this.onCategory.TabStop = true;
            this.onCategory.Text = "Sort on Category";
            this.onCategory.UseVisualStyleBackColor = true;
            this.onCategory.CheckedChanged += new System.EventHandler(this.onCategory_CheckedChanged);
            // 
            // onModule
            // 
            this.onModule.AutoSize = true;
            this.onModule.Checked = true;
            this.onModule.Location = new System.Drawing.Point(20, 14);
            this.onModule.Name = "onModule";
            this.onModule.Size = new System.Drawing.Size(97, 17);
            this.onModule.TabIndex = 0;
            this.onModule.TabStop = true;
            this.onModule.Text = "Sort on Module";
            this.onModule.UseVisualStyleBackColor = true;
            this.onModule.CheckedChanged += new System.EventHandler(this.onModule_CheckedChanged);
            // 
            // reload
            // 
            this.reload.Location = new System.Drawing.Point(275, 340);
            this.reload.Name = "reload";
            this.reload.Size = new System.Drawing.Size(75, 23);
            this.reload.TabIndex = 2;
            this.reload.Text = "Reload";
            this.reload.UseVisualStyleBackColor = true;
            this.reload.Click += new System.EventHandler(this.reload_Click);
            // 
            // TraceConfig
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(362, 378);
            this.Controls.Add(this.reload);
            this.Controls.Add(this.SortOrder);
            this.Controls.Add(this.traceTreeView);
            this.Name = "TraceConfig";
            this.Text = "Trace Configuration";
            this.Load += new System.EventHandler(this.TraceConfig_Load);
            this.SortOrder.ResumeLayout(false);
            this.SortOrder.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView traceTreeView;
        private System.Windows.Forms.GroupBox SortOrder;
        private System.Windows.Forms.RadioButton onCategory;
        private System.Windows.Forms.RadioButton onModule;
        private System.Windows.Forms.Button reload;
    }
}