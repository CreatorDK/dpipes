namespace MessangerCS
{
    partial class FormMain
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
            label_Handle = new Label();
            label_HandleValue = new Label();
            label_Role = new Label();
            label_RoleValue = new Label();
            listBox_Receive = new ListBox();
            textBox_Message = new TextBox();
            label_PipeType = new Label();
            comboBox_PipeType = new ComboBox();
            label_MessageType = new Label();
            comboBox_MassageType = new ComboBox();
            button_Send = new Button();
            toolStrip1 = new ToolStrip();
            toolStripLabel_Status = new ToolStripLabel();
            toolStripButton_Start = new ToolStripButton();
            toolStripButton_Stop = new ToolStripButton();
            toolStripButton_ClientStart = new ToolStripButton();
            toolStripButton_Connect = new ToolStripButton();
            comboBox_RequestSize = new ComboBox();
            button_Request1 = new Button();
            button_SendFile = new Button();
            toolStrip1.SuspendLayout();
            SuspendLayout();
            // 
            // label_Handle
            // 
            label_Handle.AutoSize = true;
            label_Handle.Location = new Point(12, 9);
            label_Handle.Name = "label_Handle";
            label_Handle.Size = new Size(60, 20);
            label_Handle.TabIndex = 0;
            label_Handle.Text = "Handle:";
            // 
            // label_HandleValue
            // 
            label_HandleValue.AutoSize = true;
            label_HandleValue.Location = new Point(78, 9);
            label_HandleValue.Name = "label_HandleValue";
            label_HandleValue.Size = new Size(274, 20);
            label_HandleValue.TabIndex = 1;
            label_HandleValue.Text = "0000000000000000:::0000000000000000";
            // 
            // label_Role
            // 
            label_Role.AutoSize = true;
            label_Role.Location = new Point(453, 9);
            label_Role.Name = "label_Role";
            label_Role.Size = new Size(82, 20);
            label_Role.TabIndex = 2;
            label_Role.Text = "DPipe role:";
            // 
            // label_RoleValue
            // 
            label_RoleValue.AutoSize = true;
            label_RoleValue.Location = new Point(541, 9);
            label_RoleValue.Name = "label_RoleValue";
            label_RoleValue.Size = new Size(69, 20);
            label_RoleValue.TabIndex = 3;
            label_RoleValue.Text = "Innitiator";
            // 
            // listBox_Receive
            // 
            listBox_Receive.FormattingEnabled = true;
            listBox_Receive.Location = new Point(12, 32);
            listBox_Receive.Name = "listBox_Receive";
            listBox_Receive.Size = new Size(598, 264);
            listBox_Receive.TabIndex = 4;
            // 
            // textBox_Message
            // 
            textBox_Message.Location = new Point(12, 307);
            textBox_Message.Name = "textBox_Message";
            textBox_Message.Size = new Size(598, 27);
            textBox_Message.TabIndex = 5;
            // 
            // label_PipeType
            // 
            label_PipeType.AutoSize = true;
            label_PipeType.Location = new Point(12, 349);
            label_PipeType.Name = "label_PipeType";
            label_PipeType.Size = new Size(74, 20);
            label_PipeType.TabIndex = 6;
            label_PipeType.Text = "Pipe type:";
            // 
            // comboBox_PipeType
            // 
            comboBox_PipeType.DropDownStyle = ComboBoxStyle.DropDownList;
            comboBox_PipeType.Font = new Font("Segoe UI", 8F);
            comboBox_PipeType.FormattingEnabled = true;
            comboBox_PipeType.Items.AddRange(new object[] { "Anonymous", "Named" });
            comboBox_PipeType.Location = new Point(121, 344);
            comboBox_PipeType.Name = "comboBox_PipeType";
            comboBox_PipeType.Size = new Size(124, 25);
            comboBox_PipeType.TabIndex = 7;
            // 
            // label_MessageType
            // 
            label_MessageType.AutoSize = true;
            label_MessageType.Location = new Point(251, 349);
            label_MessageType.Name = "label_MessageType";
            label_MessageType.Size = new Size(103, 20);
            label_MessageType.TabIndex = 8;
            label_MessageType.Text = "Message type:";
            // 
            // comboBox_MassageType
            // 
            comboBox_MassageType.DropDownStyle = ComboBoxStyle.DropDownList;
            comboBox_MassageType.Font = new Font("Segoe UI", 8F);
            comboBox_MassageType.FormattingEnabled = true;
            comboBox_MassageType.Items.AddRange(new object[] { "Message", "Info", "Warning", "Error" });
            comboBox_MassageType.Location = new Point(359, 343);
            comboBox_MassageType.Name = "comboBox_MassageType";
            comboBox_MassageType.Size = new Size(123, 25);
            comboBox_MassageType.TabIndex = 9;
            // 
            // button_Send
            // 
            button_Send.Font = new Font("Segoe UI", 8F);
            button_Send.Location = new Point(488, 338);
            button_Send.Name = "button_Send";
            button_Send.Size = new Size(122, 31);
            button_Send.TabIndex = 10;
            button_Send.Text = "Send Message";
            button_Send.UseVisualStyleBackColor = true;
            button_Send.Click += button_Send_Click;
            // 
            // toolStrip1
            // 
            toolStrip1.Dock = DockStyle.Bottom;
            toolStrip1.ImageScalingSize = new Size(20, 20);
            toolStrip1.Items.AddRange(new ToolStripItem[] { toolStripLabel_Status, toolStripButton_Start, toolStripButton_Stop, toolStripButton_ClientStart, toolStripButton_Connect });
            toolStrip1.Location = new Point(0, 378);
            toolStrip1.Name = "toolStrip1";
            toolStrip1.Size = new Size(957, 39);
            toolStrip1.TabIndex = 11;
            toolStrip1.Text = "toolStrip1";
            // 
            // toolStripLabel_Status
            // 
            toolStripLabel_Status.Name = "toolStripLabel_Status";
            toolStripLabel_Status.Size = new Size(84, 36);
            toolStripLabel_Status.Text = "Not started";
            // 
            // toolStripButton_Start
            // 
            toolStripButton_Start.Alignment = ToolStripItemAlignment.Right;
            toolStripButton_Start.DisplayStyle = ToolStripItemDisplayStyle.Image;
            toolStripButton_Start.Image = (Image)resources.GetObject("toolStripButton_Start.Image");
            toolStripButton_Start.ImageScaling = ToolStripItemImageScaling.None;
            toolStripButton_Start.ImageTransparentColor = Color.Magenta;
            toolStripButton_Start.Name = "toolStripButton_Start";
            toolStripButton_Start.Size = new Size(36, 36);
            toolStripButton_Start.Text = "Start";
            toolStripButton_Start.Click += toolStripButton_Start_Click;
            // 
            // toolStripButton_Stop
            // 
            toolStripButton_Stop.Alignment = ToolStripItemAlignment.Right;
            toolStripButton_Stop.DisplayStyle = ToolStripItemDisplayStyle.Image;
            toolStripButton_Stop.Image = (Image)resources.GetObject("toolStripButton_Stop.Image");
            toolStripButton_Stop.ImageScaling = ToolStripItemImageScaling.None;
            toolStripButton_Stop.ImageTransparentColor = Color.Magenta;
            toolStripButton_Stop.Name = "toolStripButton_Stop";
            toolStripButton_Stop.Size = new Size(36, 36);
            toolStripButton_Stop.Text = "Stop";
            toolStripButton_Stop.Visible = false;
            toolStripButton_Stop.Click += toolStripButton_Stop_Click;
            // 
            // toolStripButton_ClientStart
            // 
            toolStripButton_ClientStart.Alignment = ToolStripItemAlignment.Right;
            toolStripButton_ClientStart.DisplayStyle = ToolStripItemDisplayStyle.Image;
            toolStripButton_ClientStart.Image = (Image)resources.GetObject("toolStripButton_ClientStart.Image");
            toolStripButton_ClientStart.ImageScaling = ToolStripItemImageScaling.None;
            toolStripButton_ClientStart.ImageTransparentColor = Color.Magenta;
            toolStripButton_ClientStart.Name = "toolStripButton_ClientStart";
            toolStripButton_ClientStart.Size = new Size(36, 36);
            toolStripButton_ClientStart.Text = "Run Client";
            toolStripButton_ClientStart.Visible = false;
            toolStripButton_ClientStart.Click += toolStripButton_ClientStart_Click;
            // 
            // toolStripButton_Connect
            // 
            toolStripButton_Connect.Alignment = ToolStripItemAlignment.Right;
            toolStripButton_Connect.DisplayStyle = ToolStripItemDisplayStyle.Image;
            toolStripButton_Connect.Image = (Image)resources.GetObject("toolStripButton_Connect.Image");
            toolStripButton_Connect.ImageScaling = ToolStripItemImageScaling.None;
            toolStripButton_Connect.ImageTransparentColor = Color.Magenta;
            toolStripButton_Connect.Name = "toolStripButton_Connect";
            toolStripButton_Connect.Size = new Size(36, 36);
            toolStripButton_Connect.Text = "Connect";
            toolStripButton_Connect.Visible = false;
            toolStripButton_Connect.Click += toolStripButton_Connect_Click;
            // 
            // comboBox_RequestSize
            // 
            comboBox_RequestSize.DropDownStyle = ComboBoxStyle.DropDownList;
            comboBox_RequestSize.FormattingEnabled = true;
            comboBox_RequestSize.Items.AddRange(new object[] { "1 MB", "2 MB", "4 MB", "8 MB", "16 MB", "32 MB" });
            comboBox_RequestSize.Location = new Point(616, 32);
            comboBox_RequestSize.Name = "comboBox_RequestSize";
            comboBox_RequestSize.Size = new Size(177, 28);
            comboBox_RequestSize.TabIndex = 12;
            // 
            // button_Request1
            // 
            button_Request1.Enabled = false;
            button_Request1.Location = new Point(799, 31);
            button_Request1.Name = "button_Request1";
            button_Request1.Size = new Size(146, 29);
            button_Request1.TabIndex = 13;
            button_Request1.Text = "Send Request";
            button_Request1.UseVisualStyleBackColor = true;
            button_Request1.Click += button_Request1_Click;
            // 
            // button_SendFile
            // 
            button_SendFile.Enabled = false;
            button_SendFile.Location = new Point(616, 66);
            button_SendFile.Name = "button_SendFile";
            button_SendFile.Size = new Size(329, 29);
            button_SendFile.TabIndex = 14;
            button_SendFile.Text = "Send File";
            button_SendFile.UseVisualStyleBackColor = true;
            button_SendFile.Click += button_SendFile_Click;
            // 
            // FormMain
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(957, 417);
            Controls.Add(button_SendFile);
            Controls.Add(button_Request1);
            Controls.Add(comboBox_RequestSize);
            Controls.Add(toolStrip1);
            Controls.Add(button_Send);
            Controls.Add(comboBox_MassageType);
            Controls.Add(label_MessageType);
            Controls.Add(comboBox_PipeType);
            Controls.Add(label_PipeType);
            Controls.Add(textBox_Message);
            Controls.Add(listBox_Receive);
            Controls.Add(label_RoleValue);
            Controls.Add(label_Role);
            Controls.Add(label_HandleValue);
            Controls.Add(label_Handle);
            FormBorderStyle = FormBorderStyle.FixedSingle;
            Name = "FormMain";
            Text = "DPipe Massanger CS";
            FormClosing += FormMain_FormClosing;
            Load += FormMain_Load;
            toolStrip1.ResumeLayout(false);
            toolStrip1.PerformLayout();
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label_Handle;
        private Label label_HandleValue;
        private Label label_Role;
        private Label label_RoleValue;
        private ListBox listBox_Receive;
        private TextBox textBox_Message;
        private Label label_PipeType;
        private ComboBox comboBox_PipeType;
        private Label label_MessageType;
        private ComboBox comboBox_MassageType;
        private Button button_Send;
        private ToolStrip toolStrip1;
        private ToolStripLabel toolStripLabel_Status;
        private ToolStripButton toolStripButton_Start;
        private ToolStripButton toolStripButton_Stop;
        private ToolStripButton toolStripButton_ClientStart;
        private ToolStripButton toolStripButton_Connect;
        private ComboBox comboBox_RequestSize;
        private Button button_Request1;
        private Button button_SendFile;
    }
}
