using CreatorDK.Diagnostics.Win32;
using CreatorDK.DTE;
using CreatorDK.IO.DPipes;
using CreatorDK.IO.DPipes.Win32;
using System.Windows.Forms;

namespace MessangerCS
{
    public partial class FormMain : Form
    {
        string[] _args;

        private bool _isClient = false;

        private DPipe? _dpipe = null;
        private DPUser? _dpuser = null;
        private DPFileTransporter? _dpFileTransporter = null;

        public FormMain(string[] args)
        {
            _args = args;
            InitializeComponent();
        }

        private void FormMain_Load(object sender, EventArgs e)
        {
            if (_args.Contains("/attach"))
                Thread.Sleep(3000);

            if (_args.Length > 2 && _args[1] == "client")
            {
                _isClient = true;
                comboBox_PipeType.Enabled = false;
                if (_args.Length > 1)
                {
                    label_RoleValue.Text = "Client";
                    toolStripLabel_Status.Text = "Not Connected";
                    label_HandleValue.Text = _args[2];
                    toolStripButton_Start.Visible = false;
                    toolStripButton_Connect.Visible = true;
                }
            }

            comboBox_PipeType.SelectedIndex = 0;
            comboBox_MassageType.SelectedIndex = 0;
            comboBox_RequestSize.SelectedIndex = 0;
        }

        private void toolStripButton_Start_Click(object sender, EventArgs e)
        {
            if (comboBox_PipeType.Text == "Anonymous")
                _dpipe = DPipeBuilder.Create(DP_TYPE.ANONYMOUS_PIPE);
            else
                _dpipe = DPipeBuilder.Create(DP_TYPE.NAMED_PIPE);

            _dpuser = new DPUser(_dpipe, false);

            _dpFileTransporter = new DPFileTransporter(_dpuser, 10);
            _dpFileTransporter.OnFileReceived = OnFileReceived;

            _dpuser.OnOtherSideConnecting = OnOtherSideConnect;
            _dpuser.OnOtherSideDisconnecting = OnOtherSideDisconnect;
            _dpuser.OnMessageStringReceived = OnMessageStringReceived;
            _dpuser.OnInfoStringReceived = OnInfoStringReceived;
            _dpuser.OnWarningStringReceived = OnWarningStringReceived;
            _dpuser.OnErrorStringReceived = OnErrorStringReceived;

            _dpuser.SetHandler(1, Handler1);

            _dpipe.StartWin32();

            var handleString = _dpipe.GetHandleString();
            label_HandleValue.Text = handleString;

            toolStripButton_Start.Visible = false;
            toolStripButton_Stop.Visible = true;
            toolStripButton_ClientStart.Visible = true;
            comboBox_PipeType.Enabled = false;
            label_RoleValue.Text = "Innitiator";

            toolStripLabel_Status.Text = "Waiting for connection";
        }

        private void toolStripButton_Stop_Click(object sender, EventArgs e)
        {
            _dpipe?.Disconnect();

            if (_dpuser == null)
                return;

            _dpuser.OnOtherSideConnecting = null;
            _dpuser.OnOtherSideDisconnecting = null;
            _dpuser.OnMessageStringReceived = null;
            _dpuser.OnInfoStringReceived = null;
            _dpuser.OnWarningStringReceived = null;
            _dpuser.OnErrorStringReceived = null;

            _dpuser.RemoveHandler(10);

            toolStripButton_Stop.Visible = false;
            toolStripButton_ClientStart.Visible = false;
            toolStripButton_Connect.Visible = false;
            toolStripButton_Start.Visible = true;
            toolStripLabel_Status.Text = "Not stated";
            label_HandleValue.Text = "0000000000000000:::0000000000000000";

            if (!_isClient)
                comboBox_PipeType.Enabled = true;
        }

        private void OnOtherSideConnect(PacketHeader header)
        {
            if (_dpuser == null)
                return;

            var message = _dpuser.GetString(header);

            toolStripLabel_Status.Text = "Client connected";

            if (message.Length > 0)
            {
                string stringText = $"[Connection]: {message}";

                if (listBox_Receive.InvokeRequired)
                    listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(message); }));
                else
                    listBox_Receive.Items.Add(message);
            }

            if (button_Send.InvokeRequired)
                button_Send.BeginInvoke(new Action(() =>
                {
                    button_Send.Enabled = true;

                    button_Request1.Enabled = true;
                    button_SendFile.Enabled = true;
                }));
            else
            {
                button_Send.Enabled = true;

                button_Request1.Enabled = true;
                button_SendFile.Enabled = true;
            }
        }

        private void OnOtherSideDisconnect(PacketHeader header)
        {
            if (_dpuser == null)
                return;

            var message = _dpuser.GetString(header);

            if (message.Length > 0)
            {
                string stringText = $"[Disconnection]: {message}";

                if (listBox_Receive.InvokeRequired)
                    listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(message); }));
                else
                    listBox_Receive.Items.Add(message);
            }

            if (button_Send.InvokeRequired)
                button_Send.BeginInvoke(new Action(() =>
                {

                    toolStripButton_Stop.Visible = false;
                    toolStripButton_ClientStart.Visible = false;
                    toolStripButton_Connect.Visible = false;
                    toolStripButton_Start.Visible = true;
                    toolStripLabel_Status.Text = "Disconnected";
                    button_Send.Enabled = false;

                    button_Request1.Enabled = false;
                    button_SendFile.Enabled = false;

                    label_HandleValue.Text = "0000000000000000:::0000000000000000";

                    if (!_isClient)
                        comboBox_PipeType.Enabled = true;
                }));
            else
            {
                toolStripButton_Stop.Visible = false;
                toolStripButton_ClientStart.Visible = false;
                toolStripButton_Connect.Visible = false;
                toolStripButton_Start.Visible = true;
                toolStripLabel_Status.Text = "Disconnected";
                button_Send.Enabled = false;

                button_Request1.Enabled = false;
                button_SendFile.Enabled = false;

                label_HandleValue.Text = "0000000000000000:::0000000000000000";

                if (!_isClient)
                    comboBox_PipeType.Enabled = true;
            }
        }

        private void OnMessageStringReceived(string message)
        {
            string stringText = $"[Message]: {message}";

            if (listBox_Receive.InvokeRequired)
                listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(stringText); }));
            else
                listBox_Receive.Items.Add(stringText);
        }

        private void OnInfoStringReceived(string message)
        {
            string stringText = $"[Message]: {message}";

            MessageBox.Show(message, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);

            if (listBox_Receive.InvokeRequired)
                listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(stringText); }));
            else
                listBox_Receive.Items.Add(stringText);
        }

        private void OnWarningStringReceived(string message)
        {
            string stringText = $"[Warning]: {message}";

            MessageBox.Show(message, "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);

            if (listBox_Receive.InvokeRequired)
                listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(stringText); }));
            else
                listBox_Receive.Items.Add(stringText);
        }

        private void OnErrorStringReceived(string message)
        {
            string stringText = $"[Error]: {message}";

            MessageBox.Show(message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);

            if (listBox_Receive.InvokeRequired)
                listBox_Receive.BeginInvoke(new Action(() => { listBox_Receive.Items.Add(stringText); }));
            else
                listBox_Receive.Items.Add(stringText);
        }

        private void toolStripButton_ClientStart_Click(object sender, EventArgs e)
        {
            if (_dpipe == null)
                return;

            var directory = Path.GetDirectoryName(_args[0]);
            OpenFileDialog openFileDialog1 = new OpenFileDialog();
            openFileDialog1.Title = "Select DPipeClient application";
            openFileDialog1.InitialDirectory = directory;
            openFileDialog1.Filter = "Excecutable Files (*.exe)|*.exe";
            //openFileDialog1.ShowHelp = true;
            var result = openFileDialog1.ShowDialog();
            if (result == DialogResult.OK)
            {
                ProcessWin32 process = new ProcessWin32()
                {
                    CreationFlags = CreationFlags.CREATE_NEW_CONSOLE,
                    InheritHandles = true
                };

                process.ApplicationName = openFileDialog1.FileName;
                process.CommandLine = $"{openFileDialog1.FileName} client {_dpipe.GetHandleString()} /attach";
                try
                {
                    process.Start();
                    var processId = process.Id;
                    var processDTE = ProcessDTE.GetProcess(processId);
                    processDTE?.Attach();

                    toolStripButton_ClientStart.Visible = false;
                }

                catch (Exception ex)
                {
                    MessageBox.Show($"Unable to create process: {ex}");
                }
            }
        }

        private void toolStripButton_Connect_Click(object sender, EventArgs e)
        {
            toolStripButton_Connect.Visible = false;
            toolStripButton_Stop.Visible = true;

            var handle = label_HandleValue.Text;
            var message = textBox_Message.Text;

            _dpipe = DPipeBuilder.Create(handle);
            _dpuser = new DPUser(_dpipe, false);

            _dpFileTransporter = new DPFileTransporter(_dpuser, 10);
            _dpFileTransporter.OnFileReceived = OnFileReceived;

            _dpuser.OnOtherSideDisconnecting = OnOtherSideDisconnect;
            _dpuser.OnMessageStringReceived = OnMessageStringReceived;
            _dpuser.OnInfoStringReceived = OnInfoStringReceived;
            _dpuser.OnWarningStringReceived = OnWarningStringReceived;
            _dpuser.OnErrorStringReceived = OnErrorStringReceived;

            _dpuser.SetHandler(1, Handler1);

            try
            {
                _dpuser.Connect(handle, message);
                toolStripLabel_Status.Text = "Connected";

                button_Send.Enabled = true;

                button_Request1.Enabled = true;
                button_SendFile.Enabled = true;

                if (_dpipe.Type == DP_TYPE.ANONYMOUS_PIPE)
                    comboBox_PipeType.SelectedIndex = 0;
                else
                    comboBox_PipeType.SelectedIndex = 1;
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Unable to connect the pipe: {ex}", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void button_Send_Click(object sender, EventArgs e)
        {
            if (_dpuser == null)
                return;

            var type = DP_MESSAGE_TYPE.MESSAGE;

            if (comboBox_MassageType.Text == "Message")
                type = DP_MESSAGE_TYPE.MESSAGE;
            else if (comboBox_MassageType.Text == "Info")
                type = DP_MESSAGE_TYPE.MESSAGE_INFO;
            else if (comboBox_MassageType.Text == "Warning")
                type = DP_MESSAGE_TYPE.MESSAGE_WARNING;
            else if (comboBox_MassageType.Text == "Error")
                type = DP_MESSAGE_TYPE.MESSAGE_ERROR;

            _dpuser.Send(type, textBox_Message.Text);
            textBox_Message.Clear();
        }

        private void Handler1(DPReceivedRequest req)
        {
            var server = req.Server;
            var resp = req.CreateResponse();
            resp.Code = 0;
            server.SendResponse(req, resp);
        }

        private void OnFileReceived(DPFileReceiverRequest req)
        {
            Invoke(() =>
            {
                var fileName = req.FileName;

                var saveFileDialog = new SaveFileDialog();
                saveFileDialog.FileName = fileName;
                //saveFileDialog.ShowHelp = true;
                saveFileDialog.InitialDirectory = Directory.GetCurrentDirectory();
                saveFileDialog.Filter = "All files (*.*)|*.*";

                var result = saveFileDialog.ShowDialog();

                if (result == DialogResult.OK)
                {
                    var fileNameNew = saveFileDialog.FileName;

                    FileStream fileStream = File.OpenWrite(fileNameNew);

                    req.SaveFile(fileStream, 4096);

                    fileStream.Close();
                }
            });
        }

        private void button_Request1_Click(object sender, EventArgs e)
        {
            int size;

            switch (comboBox_RequestSize.Text)
            {
                case "2 MB":
                    size = 2 * 1024 * 1024; break;
                case "4 MB":
                    size = 4 * 1024 * 1024; break;
                case "8 MB":
                    size = 8 * 1024 * 1024; break;
                case "16 MB":
                    size = 16 * 1024 * 1024; break;
                case "32 MB":
                    size = 32 * 1024 * 1024; break;
                default:
                    size = 1 * 1024 * 1024; break;
            }

            byte[] buffer = new byte[size];

            _dpuser?.SendRequest(1, buffer);
        }

        private void button_SendFile_Click(object sender, EventArgs e)
        {
            var openFileDialog = new OpenFileDialog();
            openFileDialog.InitialDirectory = Directory.GetCurrentDirectory();
            //openFileDialog.ShowHelp = true;
            openFileDialog.Filter = "All files (*.*)|*.*";
            var resuslt = openFileDialog.ShowDialog();

            if (resuslt == DialogResult.OK)
            {
                var fileName = openFileDialog.SafeFileName;

                FileStream file = File.OpenRead(fileName);

                _dpFileTransporter?.SendFile(file, fileName, 4096);

                file.Close();
            }
        }

        private void FormMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            _dpipe?.Disconnect();
        }
    }
}
