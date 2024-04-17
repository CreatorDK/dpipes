#pragma once
#include <shared_mutex>
#include <memory>
#include <map>
#include <dpipe.h>
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

using namespace crdk::dpipes;

namespace MessangerCPP {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;
	using namespace System::Diagnostics;

	/// <summary>
	/// —водка дл€ FormMain
	/// </summary>
	public ref class FormMain : public System::Windows::Forms::Form
	{
	private: array<String^>^ _args;
	private: bool _isClient = false;
	private: IDPipe* _dpipe = nullptr;
	private: DPUser* _dpuser = nullptr;
	private: DPFileTransporter* _dpfileTransporter = nullptr;

	private: unsigned long long selectedMemoryAddress;

	private: System::Windows::Forms::ContextMenuStrip^ contextMenuStrip_ListView;

	private: System::Windows::Forms::ToolStripMenuItem^ clearToolStripMenuItem;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_Up;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_down;
	private: System::Windows::Forms::DataGridView^ dataGridView_MemoryAllocations;

	private: System::Windows::Forms::ContextMenuStrip^ contextMenuStrip_Memory;
	private: System::Windows::Forms::ToolStripMenuItem^ freeToolStripMenuItem;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ ColumnMemoryAddress;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^ ColumnSize;

	private: System::Windows::Forms::Button^ button_Refresh;
	private: System::Windows::Forms::CheckBox^ checkBox_KeepRequestData;
	private: System::Windows::Forms::CheckBox^ checkBox_KeepResponseData;

	private: static System::Threading::Mutex^ dictionaryMutex = (gcnew System::Threading::Mutex());
	private: static System::Collections::Generic::Dictionary<IntPtr, FormMain^>^ pipeFormDictionary = (gcnew System::Collections::Generic::Dictionary<IntPtr, FormMain^>());

	private: static System::Threading::Mutex^ heapAllocationsMutex = (gcnew System::Threading::Mutex());
	private: System::Windows::Forms::Label^ label_MemoryAllocated;
	private: System::Windows::Forms::Label^ label_MemoryAllocatedValue;
	private: System::Windows::Forms::ComboBox^ comboBox_RequestSize;

	private: System::Windows::Forms::Button^ button_Request1;
	private: System::Windows::Forms::Button^ button_SendFile;
	private: System::Windows::Forms::ToolStripMenuItem^ freeAllToolStripMenuItem;

	private: static System::Collections::Generic::List<FormMain^>^ heapAllocationSubcriptForms = (gcnew System::Collections::Generic::List<FormMain^>());

	private: static void AddPair(IDPipe* dpipe, FormMain^ form) {
		dictionaryMutex->WaitOne();
		if (!pipeFormDictionary->ContainsKey((IntPtr)dpipe))
			pipeFormDictionary->Add((IntPtr)dpipe, form);
		dictionaryMutex->ReleaseMutex();
	}

	private: static void RemovePair(IDPipe* dpipe) {
		dictionaryMutex->WaitOne();
		if (pipeFormDictionary->ContainsKey((IntPtr)dpipe)) {
			pipeFormDictionary->Remove((IntPtr)dpipe);
		}
		dictionaryMutex->ReleaseMutex();
	}

	private: static FormMain^ GetInstance(IDPipe* dpipe) {
		if (pipeFormDictionary->ContainsKey((IntPtr)dpipe)) {
			return pipeFormDictionary[(IntPtr)dpipe];
		}
		else
			return nullptr;
	}


	public: static void OnMemoryAllocationStatic(void* address, DWORD size) {

		for each (FormMain^ form in heapAllocationSubcriptForms) {
			form->OnMemoryAllocation(address, size);
		}
	}

	public: static void OnMemoryDeallocationStatic(void* address, DWORD size) {

		for each (FormMain ^ form in heapAllocationSubcriptForms) {
			form->OnMemoryDeallocation(address, size);
		}
	}

	private: static void SubscribeDataAllocations(FormMain^ form) {
		HeapAllocatedData::SetMemoryAllocatedCallback(OnMemoryAllocationStatic);
		HeapAllocatedData::SetMemoryDeallocatedCallback(OnMemoryDeallocationStatic);

		heapAllocationsMutex->WaitOne();

			heapAllocationSubcriptForms->Add(form);

		heapAllocationsMutex->ReleaseMutex();
	}

	private: static void UnsubscribeDataAllocations(FormMain^ form) {
		HeapAllocatedData::SetMemoryAllocatedCallback(OnMemoryAllocationStatic);
		HeapAllocatedData::SetMemoryDeallocatedCallback(OnMemoryDeallocationStatic);

		heapAllocationsMutex->WaitOne();

			heapAllocationSubcriptForms->Remove(form);

		heapAllocationsMutex->ReleaseMutex();
	}

	public:
		FormMain(array<String^>^ args)
		{
			_args = args;
			InitializeComponent();

			SubscribeDataAllocations(this);

			comboBox_PipeType->SelectedIndex = 0;
			comboBox_MassageType->SelectedIndex = 0;
			comboBox_RequestSize->SelectedIndex = 0;

			if (args->Length > 2 && args[1] == "client") {
				_isClient = true;
				comboBox_PipeType->Enabled = false;

				if (args->Length > 1) {
					label_RoleValue->Text = "Client";
					toolStripLabel_Status->Text = "Not Connected";
					label_HandleValue->Text = args[2];
					toolStripButton_Start->Visible = false;
					toolStripButton_Connect->Visible = true;
				}
			}
		}

	protected:
		/// <summary>
		/// ќсвободить все используемые ресурсы.
		/// </summary>
		~FormMain()
		{
			UnsubscribeDataAllocations(this);

			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListBox^ listBox_Receive;
	private: System::Windows::Forms::TextBox^ textBox_Message;
	protected:

	private: System::Windows::Forms::Button^ button_Send;
	private: System::Windows::Forms::ToolStrip^ toolStrip_Main;

	private: System::Windows::Forms::Label^ label_MessageType;
	private: System::Windows::Forms::ComboBox^ comboBox_MassageType;

	private: System::Windows::Forms::ToolStripLabel^ toolStripLabel_Status;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_ClientStart;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_Start;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_Stop;
	private: System::Windows::Forms::Label^ label_Role;
	private: System::Windows::Forms::ToolStripButton^ toolStripButton_Connect;
	private: System::Windows::Forms::Label^ label_RoleValue;
	private: System::Windows::Forms::Label^ label_Handle;
	private: System::Windows::Forms::Label^ label_HandleValue;
	private: System::Windows::Forms::Label^ label_PipeType;
	private: System::Windows::Forms::ComboBox^ comboBox_PipeType;
	private: System::ComponentModel::IContainer^ components;


	private:
		/// <summary>
		/// ќб€зательна€ переменна€ конструктора.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// “ребуемый метод дл€ поддержки конструктора Ч не измен€йте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(FormMain::typeid));
			this->listBox_Receive = (gcnew System::Windows::Forms::ListBox());
			this->contextMenuStrip_ListView = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->clearToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->textBox_Message = (gcnew System::Windows::Forms::TextBox());
			this->button_Send = (gcnew System::Windows::Forms::Button());
			this->toolStrip_Main = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripLabel_Status = (gcnew System::Windows::Forms::ToolStripLabel());
			this->toolStripButton_Up = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton_Start = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton_down = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton_Stop = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton_ClientStart = (gcnew System::Windows::Forms::ToolStripButton());
			this->toolStripButton_Connect = (gcnew System::Windows::Forms::ToolStripButton());
			this->label_MessageType = (gcnew System::Windows::Forms::Label());
			this->comboBox_MassageType = (gcnew System::Windows::Forms::ComboBox());
			this->label_Role = (gcnew System::Windows::Forms::Label());
			this->label_RoleValue = (gcnew System::Windows::Forms::Label());
			this->label_Handle = (gcnew System::Windows::Forms::Label());
			this->label_HandleValue = (gcnew System::Windows::Forms::Label());
			this->label_PipeType = (gcnew System::Windows::Forms::Label());
			this->comboBox_PipeType = (gcnew System::Windows::Forms::ComboBox());
			this->dataGridView_MemoryAllocations = (gcnew System::Windows::Forms::DataGridView());
			this->ColumnMemoryAddress = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->ColumnSize = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->contextMenuStrip_Memory = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->freeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->button_Refresh = (gcnew System::Windows::Forms::Button());
			this->checkBox_KeepRequestData = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox_KeepResponseData = (gcnew System::Windows::Forms::CheckBox());
			this->label_MemoryAllocated = (gcnew System::Windows::Forms::Label());
			this->label_MemoryAllocatedValue = (gcnew System::Windows::Forms::Label());
			this->comboBox_RequestSize = (gcnew System::Windows::Forms::ComboBox());
			this->button_Request1 = (gcnew System::Windows::Forms::Button());
			this->button_SendFile = (gcnew System::Windows::Forms::Button());
			this->freeAllToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->contextMenuStrip_ListView->SuspendLayout();
			this->toolStrip_Main->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView_MemoryAllocations))->BeginInit();
			this->contextMenuStrip_Memory->SuspendLayout();
			this->SuspendLayout();
			// 
			// listBox_Receive
			// 
			this->listBox_Receive->ContextMenuStrip = this->contextMenuStrip_ListView;
			this->listBox_Receive->FormattingEnabled = true;
			this->listBox_Receive->ItemHeight = 16;
			this->listBox_Receive->Location = System::Drawing::Point(12, 32);
			this->listBox_Receive->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->listBox_Receive->Name = L"listBox_Receive";
			this->listBox_Receive->Size = System::Drawing::Size(599, 276);
			this->listBox_Receive->TabIndex = 0;
			// 
			// contextMenuStrip_ListView
			// 
			this->contextMenuStrip_ListView->ImageScalingSize = System::Drawing::Size(20, 20);
			this->contextMenuStrip_ListView->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->clearToolStripMenuItem });
			this->contextMenuStrip_ListView->Name = L"contextMenuStrip1";
			this->contextMenuStrip_ListView->Size = System::Drawing::Size(113, 28);
			// 
			// clearToolStripMenuItem
			// 
			this->clearToolStripMenuItem->Name = L"clearToolStripMenuItem";
			this->clearToolStripMenuItem->Size = System::Drawing::Size(112, 24);
			this->clearToolStripMenuItem->Text = L"Clear";
			this->clearToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormMain::clearToolStripMenuItem_Click);
			// 
			// textBox_Message
			// 
			this->textBox_Message->Location = System::Drawing::Point(12, 325);
			this->textBox_Message->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_Message->Name = L"textBox_Message";
			this->textBox_Message->Size = System::Drawing::Size(599, 20);
			this->textBox_Message->TabIndex = 1;
			// 
			// button_Send
			// 
			this->button_Send->Enabled = false;
			this->button_Send->Location = System::Drawing::Point(491, 353);
			this->button_Send->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_Send->Name = L"button_Send";
			this->button_Send->Size = System::Drawing::Size(119, 28);
			this->button_Send->TabIndex = 2;
			this->button_Send->Text = L"Send Message";
			this->button_Send->UseVisualStyleBackColor = true;
			this->button_Send->Click += gcnew System::EventHandler(this, &FormMain::button_Send_Click);
			// 
			// toolStrip_Main
			// 
			this->toolStrip_Main->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->toolStrip_Main->ImageScalingSize = System::Drawing::Size(20, 20);
			this->toolStrip_Main->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(7) {
				this->toolStripLabel_Status,
					this->toolStripButton_Up, this->toolStripButton_Start, this->toolStripButton_down, this->toolStripButton_Stop, this->toolStripButton_ClientStart,
					this->toolStripButton_Connect
			});
			this->toolStrip_Main->Location = System::Drawing::Point(0, 394);
			this->toolStrip_Main->Name = L"toolStrip_Main";
			this->toolStrip_Main->Size = System::Drawing::Size(1006, 39);
			this->toolStrip_Main->TabIndex = 3;
			this->toolStrip_Main->Text = L"toolStrip1";
			// 
			// toolStripLabel_Status
			// 
			this->toolStripLabel_Status->Name = L"toolStripLabel_Status";
			this->toolStripLabel_Status->Size = System::Drawing::Size(84, 36);
			this->toolStripLabel_Status->Text = L"Not started";
			// 
			// toolStripButton_Up
			// 
			this->toolStripButton_Up->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_Up->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_Up->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_Up.Image")));
			this->toolStripButton_Up->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_Up->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_Up->Name = L"toolStripButton_Up";
			this->toolStripButton_Up->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_Up->Text = L"Up";
			this->toolStripButton_Up->Visible = false;
			this->toolStripButton_Up->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_Up_Click);
			// 
			// toolStripButton_Start
			// 
			this->toolStripButton_Start->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_Start->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_Start->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_Start.Image")));
			this->toolStripButton_Start->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_Start->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_Start->Name = L"toolStripButton_Start";
			this->toolStripButton_Start->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_Start->Text = L"Start";
			this->toolStripButton_Start->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_Start_Click);
			// 
			// toolStripButton_down
			// 
			this->toolStripButton_down->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_down->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_down->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_down.Image")));
			this->toolStripButton_down->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_down->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_down->Name = L"toolStripButton_down";
			this->toolStripButton_down->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_down->Text = L"Down";
			this->toolStripButton_down->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_down_Click);
			// 
			// toolStripButton_Stop
			// 
			this->toolStripButton_Stop->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_Stop->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_Stop->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_Stop.Image")));
			this->toolStripButton_Stop->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_Stop->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_Stop->Name = L"toolStripButton_Stop";
			this->toolStripButton_Stop->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_Stop->Text = L"Stop";
			this->toolStripButton_Stop->Visible = false;
			this->toolStripButton_Stop->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_Stop_Click);
			// 
			// toolStripButton_ClientStart
			// 
			this->toolStripButton_ClientStart->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_ClientStart->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_ClientStart->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_ClientStart.Image")));
			this->toolStripButton_ClientStart->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_ClientStart->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_ClientStart->Name = L"toolStripButton_ClientStart";
			this->toolStripButton_ClientStart->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_ClientStart->Text = L"Client";
			this->toolStripButton_ClientStart->Visible = false;
			this->toolStripButton_ClientStart->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_ClientStart_Click);
			// 
			// toolStripButton_Connect
			// 
			this->toolStripButton_Connect->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->toolStripButton_Connect->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
			this->toolStripButton_Connect->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripButton_Connect.Image")));
			this->toolStripButton_Connect->ImageScaling = System::Windows::Forms::ToolStripItemImageScaling::None;
			this->toolStripButton_Connect->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripButton_Connect->Name = L"toolStripButton_Connect";
			this->toolStripButton_Connect->Size = System::Drawing::Size(36, 36);
			this->toolStripButton_Connect->Text = L"Connect";
			this->toolStripButton_Connect->Visible = false;
			this->toolStripButton_Connect->Click += gcnew System::EventHandler(this, &FormMain::toolStripButton_Connect_Click);
			// 
			// label_MessageType
			// 
			this->label_MessageType->AutoSize = true;
			this->label_MessageType->Location = System::Drawing::Point(261, 361);
			this->label_MessageType->Name = L"label_MessageType";
			this->label_MessageType->Size = System::Drawing::Size(86, 15);
			this->label_MessageType->TabIndex = 4;
			this->label_MessageType->Text = L"Message type:";
			// 
			// comboBox_MassageType
			// 
			this->comboBox_MassageType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox_MassageType->FormattingEnabled = true;
			this->comboBox_MassageType->Items->AddRange(gcnew cli::array< System::Object^  >(4) { L"Message", L"Info", L"Warning", L"Error" });
			this->comboBox_MassageType->Location = System::Drawing::Point(364, 356);
			this->comboBox_MassageType->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->comboBox_MassageType->Name = L"comboBox_MassageType";
			this->comboBox_MassageType->Size = System::Drawing::Size(121, 24);
			this->comboBox_MassageType->TabIndex = 5;
			// 
			// label_Role
			// 
			this->label_Role->AutoSize = true;
			this->label_Role->Location = System::Drawing::Point(444, 9);
			this->label_Role->Name = L"label_Role";
			this->label_Role->Size = System::Drawing::Size(68, 15);
			this->label_Role->TabIndex = 6;
			this->label_Role->Text = L"DPipe role:";
			// 
			// label_RoleValue
			// 
			this->label_RoleValue->AutoSize = true;
			this->label_RoleValue->Location = System::Drawing::Point(535, 9);
			this->label_RoleValue->Name = L"label_RoleValue";
			this->label_RoleValue->Size = System::Drawing::Size(54, 15);
			this->label_RoleValue->TabIndex = 7;
			this->label_RoleValue->Text = L"Innitiator";
			// 
			// label_Handle
			// 
			this->label_Handle->AutoSize = true;
			this->label_Handle->Location = System::Drawing::Point(12, 9);
			this->label_Handle->Name = L"label_Handle";
			this->label_Handle->Size = System::Drawing::Size(50, 15);
			this->label_Handle->TabIndex = 6;
			this->label_Handle->Text = L"Handle:";
			// 
			// label_HandleValue
			// 
			this->label_HandleValue->AutoSize = true;
			this->label_HandleValue->Location = System::Drawing::Point(72, 9);
			this->label_HandleValue->Name = L"label_HandleValue";
			this->label_HandleValue->Size = System::Drawing::Size(240, 15);
			this->label_HandleValue->TabIndex = 7;
			this->label_HandleValue->Text = L"0000000000000000:::0000000000000000";
			// 
			// label_PipeType
			// 
			this->label_PipeType->AutoSize = true;
			this->label_PipeType->Location = System::Drawing::Point(12, 362);
			this->label_PipeType->Name = L"label_PipeType";
			this->label_PipeType->Size = System::Drawing::Size(60, 15);
			this->label_PipeType->TabIndex = 8;
			this->label_PipeType->Text = L"Pipe type:";
			// 
			// comboBox_PipeType
			// 
			this->comboBox_PipeType->DisplayMember = L"0";
			this->comboBox_PipeType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox_PipeType->FormattingEnabled = true;
			this->comboBox_PipeType->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Anonymous", L"Named" });
			this->comboBox_PipeType->Location = System::Drawing::Point(129, 357);
			this->comboBox_PipeType->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->comboBox_PipeType->Name = L"comboBox_PipeType";
			this->comboBox_PipeType->Size = System::Drawing::Size(121, 24);
			this->comboBox_PipeType->TabIndex = 9;
			// 
			// dataGridView_MemoryAllocations
			// 
			this->dataGridView_MemoryAllocations->AllowUserToAddRows = false;
			this->dataGridView_MemoryAllocations->BackgroundColor = System::Drawing::SystemColors::ButtonFace;
			this->dataGridView_MemoryAllocations->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dataGridView_MemoryAllocations->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
				this->ColumnMemoryAddress,
					this->ColumnSize
			});
			this->dataGridView_MemoryAllocations->ContextMenuStrip = this->contextMenuStrip_Memory;
			this->dataGridView_MemoryAllocations->Location = System::Drawing::Point(12, 434);
			this->dataGridView_MemoryAllocations->MultiSelect = false;
			this->dataGridView_MemoryAllocations->Name = L"dataGridView_MemoryAllocations";
			this->dataGridView_MemoryAllocations->ReadOnly = true;
			this->dataGridView_MemoryAllocations->RowHeadersVisible = false;
			this->dataGridView_MemoryAllocations->RowHeadersWidth = 51;
			this->dataGridView_MemoryAllocations->RowTemplate->Height = 24;
			this->dataGridView_MemoryAllocations->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dataGridView_MemoryAllocations->Size = System::Drawing::Size(598, 150);
			this->dataGridView_MemoryAllocations->TabIndex = 10;
			this->dataGridView_MemoryAllocations->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &FormMain::dataGridView_MemoryAllocations_MouseDown);
			// 
			// ColumnMemoryAddress
			// 
			this->ColumnMemoryAddress->HeaderText = L"Address";
			this->ColumnMemoryAddress->MinimumWidth = 6;
			this->ColumnMemoryAddress->Name = L"ColumnMemoryAddress";
			this->ColumnMemoryAddress->ReadOnly = true;
			this->ColumnMemoryAddress->Width = 200;
			// 
			// ColumnSize
			// 
			this->ColumnSize->HeaderText = L"Size";
			this->ColumnSize->MinimumWidth = 6;
			this->ColumnSize->Name = L"ColumnSize";
			this->ColumnSize->ReadOnly = true;
			this->ColumnSize->Width = 200;
			// 
			// contextMenuStrip_Memory
			// 
			this->contextMenuStrip_Memory->ImageScalingSize = System::Drawing::Size(20, 20);
			this->contextMenuStrip_Memory->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->freeToolStripMenuItem,
					this->freeAllToolStripMenuItem
			});
			this->contextMenuStrip_Memory->Name = L"contextMenuStrip_Memory";
			this->contextMenuStrip_Memory->Size = System::Drawing::Size(211, 80);
			this->contextMenuStrip_Memory->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &FormMain::contextMenuStrip_Memory_Opening);
			// 
			// freeToolStripMenuItem
			// 
			this->freeToolStripMenuItem->Name = L"freeToolStripMenuItem";
			this->freeToolStripMenuItem->Size = System::Drawing::Size(210, 24);
			this->freeToolStripMenuItem->Text = L"Free";
			this->freeToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormMain::freeToolStripMenuItem_Click);
			// 
			// button_Refresh
			// 
			this->button_Refresh->Location = System::Drawing::Point(492, 399);
			this->button_Refresh->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_Refresh->Name = L"button_Refresh";
			this->button_Refresh->Size = System::Drawing::Size(119, 28);
			this->button_Refresh->TabIndex = 2;
			this->button_Refresh->Text = L"Refresh";
			this->button_Refresh->UseVisualStyleBackColor = true;
			this->button_Refresh->Click += gcnew System::EventHandler(this, &FormMain::button_Refresh_Click);
			// 
			// checkBox_KeepRequestData
			// 
			this->checkBox_KeepRequestData->AutoSize = true;
			this->checkBox_KeepRequestData->Location = System::Drawing::Point(617, 32);
			this->checkBox_KeepRequestData->Name = L"checkBox_KeepRequestData";
			this->checkBox_KeepRequestData->Size = System::Drawing::Size(189, 19);
			this->checkBox_KeepRequestData->TabIndex = 11;
			this->checkBox_KeepRequestData->Text = L"Kepp Request Allocated Data";
			this->checkBox_KeepRequestData->UseVisualStyleBackColor = true;
			this->checkBox_KeepRequestData->CheckedChanged += gcnew System::EventHandler(this, &FormMain::checkBox_KeepRequestData_CheckedChanged);
			// 
			// checkBox_KeepResponseData
			// 
			this->checkBox_KeepResponseData->AutoSize = true;
			this->checkBox_KeepResponseData->Location = System::Drawing::Point(617, 57);
			this->checkBox_KeepResponseData->Name = L"checkBox_KeepResponseData";
			this->checkBox_KeepResponseData->Size = System::Drawing::Size(199, 19);
			this->checkBox_KeepResponseData->TabIndex = 12;
			this->checkBox_KeepResponseData->Text = L"Kepp Response Allocated Data";
			this->checkBox_KeepResponseData->UseVisualStyleBackColor = true;
			this->checkBox_KeepResponseData->CheckedChanged += gcnew System::EventHandler(this, &FormMain::checkBox_KeepResponseData_CheckedChanged);
			// 
			// label_MemoryAllocated
			// 
			this->label_MemoryAllocated->AutoSize = true;
			this->label_MemoryAllocated->Location = System::Drawing::Point(614, 9);
			this->label_MemoryAllocated->Name = L"label_MemoryAllocated";
			this->label_MemoryAllocated->Size = System::Drawing::Size(108, 15);
			this->label_MemoryAllocated->TabIndex = 13;
			this->label_MemoryAllocated->Text = L"Memory Allocated:";
			// 
			// label_MemoryAllocatedValue
			// 
			this->label_MemoryAllocatedValue->AutoSize = true;
			this->label_MemoryAllocatedValue->Location = System::Drawing::Point(728, 9);
			this->label_MemoryAllocatedValue->Name = L"label_MemoryAllocatedValue";
			this->label_MemoryAllocatedValue->Size = System::Drawing::Size(36, 15);
			this->label_MemoryAllocatedValue->TabIndex = 14;
			this->label_MemoryAllocatedValue->Text = L"0 MB";
			// 
			// comboBox_RequestSize
			// 
			this->comboBox_RequestSize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->comboBox_RequestSize->FormattingEnabled = true;
			this->comboBox_RequestSize->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"1 MB", L"2 MB", L"4 MB", L"8 MB",
					L"16 MB", L"32 MB"
			});
			this->comboBox_RequestSize->Location = System::Drawing::Point(617, 82);
			this->comboBox_RequestSize->Name = L"comboBox_RequestSize";
			this->comboBox_RequestSize->Size = System::Drawing::Size(199, 24);
			this->comboBox_RequestSize->TabIndex = 15;
			// 
			// button_Request1
			// 
			this->button_Request1->Enabled = false;
			this->button_Request1->Location = System::Drawing::Point(822, 83);
			this->button_Request1->Name = L"button_Request1";
			this->button_Request1->Size = System::Drawing::Size(172, 24);
			this->button_Request1->TabIndex = 16;
			this->button_Request1->Text = L"Send Request";
			this->button_Request1->UseVisualStyleBackColor = true;
			this->button_Request1->Click += gcnew System::EventHandler(this, &FormMain::button_Request1_Click);
			// 
			// button_SendFile
			// 
			this->button_SendFile->Enabled = false;
			this->button_SendFile->Location = System::Drawing::Point(617, 113);
			this->button_SendFile->Name = L"button_SendFile";
			this->button_SendFile->Size = System::Drawing::Size(377, 24);
			this->button_SendFile->TabIndex = 17;
			this->button_SendFile->Text = L"Send File";
			this->button_SendFile->UseVisualStyleBackColor = true;
			this->button_SendFile->Click += gcnew System::EventHandler(this, &FormMain::button1_Click);
			// 
			// freeAllToolStripMenuItem
			// 
			this->freeAllToolStripMenuItem->Name = L"freeAllToolStripMenuItem";
			this->freeAllToolStripMenuItem->Size = System::Drawing::Size(210, 24);
			this->freeAllToolStripMenuItem->Text = L"Free all";
			this->freeAllToolStripMenuItem->Click += gcnew System::EventHandler(this, &FormMain::freeAllToolStripMenuItem_Click);
			// 
			// FormMain
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1006, 433);
			this->Controls->Add(this->button_SendFile);
			this->Controls->Add(this->button_Request1);
			this->Controls->Add(this->comboBox_RequestSize);
			this->Controls->Add(this->label_MemoryAllocatedValue);
			this->Controls->Add(this->label_MemoryAllocated);
			this->Controls->Add(this->checkBox_KeepResponseData);
			this->Controls->Add(this->checkBox_KeepRequestData);
			this->Controls->Add(this->label_PipeType);
			this->Controls->Add(this->label_HandleValue);
			this->Controls->Add(this->label_Handle);
			this->Controls->Add(this->label_RoleValue);
			this->Controls->Add(this->label_Role);
			this->Controls->Add(this->label_MessageType);
			this->Controls->Add(this->toolStrip_Main);
			this->Controls->Add(this->button_Refresh);
			this->Controls->Add(this->button_Send);
			this->Controls->Add(this->textBox_Message);
			this->Controls->Add(this->listBox_Receive);
			this->Controls->Add(this->dataGridView_MemoryAllocations);
			this->Controls->Add(this->comboBox_PipeType);
			this->Controls->Add(this->comboBox_MassageType);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->Name = L"FormMain";
			this->Text = L"DPipe Massanger CPP";
			this->contextMenuStrip_ListView->ResumeLayout(false);
			this->toolStrip_Main->ResumeLayout(false);
			this->toolStrip_Main->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView_MemoryAllocations))->EndInit();
			this->contextMenuStrip_Memory->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

private: delegate void VoidDelegate();
private: delegate void AddItemDelegate(System::Object^ item);

private: void EnableSendButton() {
	button_Send->Enabled = true;

	button_Request1->Enabled = true;
	button_SendFile->Enabled = true;
}

private: void DisableControls() {
	toolStripButton_Stop->Visible = false;
	toolStripButton_ClientStart->Visible = false;
	toolStripButton_Connect->Visible = false;
	toolStripButton_Start->Visible = true;
	toolStripLabel_Status->Text = "Disconnected";
	button_Send->Enabled = false;

	button_Request1->Enabled = false;
	button_SendFile->Enabled = false;

	label_HandleValue->Text = "0000000000000000:::0000000000000000";

	delete _dpuser;
	delete _dpipe;

	if (!_isClient)
		comboBox_PipeType->Enabled = true;
}

private: void AddListBoxItem(System::Object^ item) {
	listBox_Receive->Items->Add(item);
}

private: static void OnClientConnect(IDPipe* pipe, PacketHeader header) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {

		formInstance->toolStripLabel_Status->Text = "Client connected";

		auto text = formInstance->_dpuser->GetWString(header);

		if (text.length() > 0) {
			String^ stringText = "[Connection]: " + gcnew String(text.c_str());
			if (formInstance->listBox_Receive->InvokeRequired)
				formInstance->listBox_Receive->BeginInvoke( gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
			else
				formInstance->listBox_Receive->Items->Add(stringText);
		}

		if (formInstance->button_Send->InvokeRequired)
			formInstance->button_Send->BeginInvoke(gcnew VoidDelegate(formInstance, &EnableSendButton));
		else {
			formInstance->button_Send->Enabled = true;

			formInstance->button_Request1->Enabled = true;
			formInstance->button_SendFile->Enabled = true;
		}
			
	}
}

private: static void OnOtherSideDisconnect(IDPipe* pipe, PacketHeader header) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {
		auto text = formInstance->_dpuser->GetWString(header);

		if (text.length() > 0) {
			String^ stringText = "[Disconnection]: " + gcnew String(text.c_str());
			if (formInstance->listBox_Receive->InvokeRequired)
				formInstance->listBox_Receive->BeginInvoke(gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
			else
				formInstance->listBox_Receive->Items->Add(stringText);
		}

		if (formInstance->button_Send->InvokeRequired)
			formInstance->button_Send->BeginInvoke(gcnew VoidDelegate(formInstance, &DisableControls));
		else
		{
			formInstance->toolStripButton_Stop->Visible = false;
			formInstance->toolStripButton_ClientStart->Visible = false;
			formInstance->toolStripButton_Connect->Visible = false;
			formInstance->toolStripButton_Start->Visible = true;
			formInstance->toolStripLabel_Status->Text = "Disconnected";
			formInstance->button_Send->Enabled = false;

			formInstance->button_Request1->Enabled = false;
			formInstance->button_SendFile->Enabled = false;

			formInstance->label_HandleValue->Text = "0000000000000000:::0000000000000000";

			delete formInstance->_dpuser;
			delete formInstance->_dpipe;

			if (!formInstance->_isClient)
				formInstance->comboBox_PipeType->Enabled = true;
		}
	}
}

private: static void OnMessageStringReceived(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {

		auto text = formInstance->_dpuser->GetWString(header, data);

		String^ stringText = "[Message]: " + gcnew String(text.c_str());
		if (formInstance->listBox_Receive->InvokeRequired)
			formInstance->listBox_Receive->BeginInvoke(gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
		else
			formInstance->listBox_Receive->Items->Add(stringText);
	}
}

private: static void OnInfoStringReceived(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {

		auto text = formInstance->_dpuser->GetWString(header, data);

		String^ receivedText = gcnew String(text.c_str());
		MessageBox::Show(receivedText, "Information", MessageBoxButtons::OK, MessageBoxIcon::Information);

		String^ stringText = "[Info]: " + receivedText;

		if (formInstance->listBox_Receive->InvokeRequired)
			formInstance->listBox_Receive->BeginInvoke(gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
		else
			formInstance->listBox_Receive->Items->Add(stringText);
	}
}

private: static void OnWarningStringReceived(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {

		auto text = formInstance->_dpuser->GetWString(header, data);

		String^ receivedText = gcnew String(text.c_str());
		MessageBox::Show(receivedText, "Warning", MessageBoxButtons::OK, MessageBoxIcon::Warning);

		String^ stringText = "[Warning]: " + receivedText;

		if (formInstance->listBox_Receive->InvokeRequired)
			formInstance->listBox_Receive->BeginInvoke(gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
		else
			formInstance->listBox_Receive->Items->Add(stringText);
	}
}

private: static void OnErrorStringReceived(IDPipe* pipe, PacketHeader header, std::shared_ptr<HeapAllocatedData> data) {

	auto formInstance = GetInstance(pipe);

	if (formInstance != nullptr) {

		auto text = formInstance->_dpuser->GetWString(header, data);

		String^ receivedText = gcnew String(text.c_str());
		MessageBox::Show(receivedText, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);

		String^ stringText = "[Error]: " + receivedText;
		if (formInstance->listBox_Receive->InvokeRequired)
			formInstance->listBox_Receive->BeginInvoke(gcnew AddItemDelegate(formInstance, &AddListBoxItem), stringText);
		else
			formInstance->listBox_Receive->Items->Add(stringText);
	}
}

private: System::Void button_Send_Click(System::Object^ sender, System::EventArgs^ e) {

	auto message = msclr::interop::marshal_as<std::wstring>(textBox_Message->Text);

	DP_MESSAGE_TYPE type = DP_MESSAGE_TYPE::MESSAGE;

	if (comboBox_MassageType->Text == "Message")
		type = DP_MESSAGE_TYPE::MESSAGE;
	else if (comboBox_MassageType->Text == "Info")
		type = DP_MESSAGE_TYPE::MESSAGE_INFO;
	else if (comboBox_MassageType->Text == "Warning")
		type = DP_MESSAGE_TYPE::MESSAGE_WARNING;
	else if (comboBox_MassageType->Text == "Error")
		type = DP_MESSAGE_TYPE::MESSAGE_ERROR;

		_dpuser->Send(type, message);
		textBox_Message->Clear();
}

private: System::Void toolStripButton_Start_Click(System::Object^ sender, System::EventArgs^ e) {

	if (comboBox_PipeType->Text == "Anonymous")
		_dpipe = DPipeBuilder::Create(DP_TYPE::ANONYMOUS_PIPE);
	
	else 
		_dpipe = DPipeBuilder::Create(DP_TYPE::NAMED_PIPE);
	

	AddPair(_dpipe, this);

	_dpuser = new DPUser(_dpipe, false);
	_dpfileTransporter = new DPFileTransporter(_dpuser, 10);
	_dpfileTransporter->SetFileReceivedCallback(HandlerFileReceive);

	_dpuser->SetOnOtherSideConnectCallback(OnClientConnect);
	_dpuser->SetOnOtherSideDisconnectCallback(OnOtherSideDisconnect);
	_dpuser->SetMessageStringReceivedHandler(OnMessageStringReceived);
	_dpuser->SetInfoStringReceivedHandler(OnInfoStringReceived);
	_dpuser->SetWarningStringReceivedHandler(OnWarningStringReceived);
	_dpuser->SetErrorStringReceivedHandler(OnErrorStringReceived);

	_dpuser->SetHandler(1, Handler1);

	_dpipe->Start();

	auto handleString = _dpipe->GetHandleString();
	label_HandleValue->Text = gcnew String(handleString.c_str());

	button_Send->Enabled = true;
	toolStripButton_Start->Visible = false;
	toolStripButton_Stop->Visible = true;
	toolStripButton_ClientStart->Visible = true;
	comboBox_PipeType->Enabled = false;
	label_RoleValue->Text = "Innitiator";

	button_Request1->Enabled = true;
	button_SendFile->Enabled = true;

	toolStripLabel_Status->Text = "Waiting for connection";
}
private: System::Void toolStripButton_Stop_Click(System::Object^ sender, System::EventArgs^ e) {
	_dpipe->Disconnect();

	_dpuser->SetOnOtherSideConnectCallback({});
	_dpuser->SetOnOtherSideConnectCallback({});
	_dpuser->SetOnOtherSideDisconnectCallback({});
	_dpuser->SetMessageStringReceivedHandler({});
	_dpuser->SetInfoStringReceivedHandler({});
	_dpuser->SetWarningStringReceivedHandler({});
	_dpuser->SetErrorStringReceivedHandler({});

	toolStripButton_Stop->Visible = false;
	toolStripButton_ClientStart->Visible = false;
	toolStripButton_Connect->Visible = false;
	toolStripButton_Start->Visible = true;
	toolStripLabel_Status->Text = "Not stated";
	label_HandleValue->Text = "0000000000000000:::0000000000000000";

	button_Request1->Enabled = false;
	button_SendFile->Enabled = false;

	delete _dpuser;
	delete _dpipe;

	if (!_isClient)
		comboBox_PipeType->Enabled = true;
}
private: System::Void RunProcess(System::String^ appFileName, std::wstring handle) {
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	DWORD creatinoFlag = CREATE_NEW_CONSOLE;

	std::wstring path = msclr::interop::marshal_as<std::wstring>(appFileName);

	std::wstring commandLine = path + L" client " + handle;
	LPWSTR lpCommand = &commandLine[0];

	try {
		CreateProcess(&path[0],			// No module name (use command line)
			lpCommand,                  // Command line
			NULL,                       // Process handle not inheritable
			NULL,                       // Thread handle not inheritable
			TRUE,                       // Set handle inheritance
			creatinoFlag,               // No creation flags
			NULL,                       // Use parent's environment block
			NULL,                       // Use parent's starting directory 
			&si,						// Pointer to STARTUPINFO structure
			&pi							// Pointer to PROCESS_INFORMATION structure
		);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	catch (System::Exception^ ex) {
		MessageBox::Show("Unable to create process: " + ex->Message);
	}
}
private: System::Void toolStripButton_ClientStart_Click(System::Object^ sender, System::EventArgs^ e) {

	auto directory = Path::GetDirectoryName(_args[0]);
	OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;
	openFileDialog1->Title = "Select DPipeClient application";
	openFileDialog1->InitialDirectory = directory;
	openFileDialog1->RestoreDirectory = false;
	openFileDialog1->Filter = "Excecutable Files (*.exe)|*.exe";
	openFileDialog1->ShowHelp = true;
	auto result = openFileDialog1->ShowDialog();
	if (result == System::Windows::Forms::DialogResult::OK) {
		auto appFileName = openFileDialog1->FileName;
		RunProcess(appFileName, _dpipe->GetHandleString());
		toolStripButton_ClientStart->Visible = false;
	}
}
private: System::Void toolStripButton_Connect_Click(System::Object^ sender, System::EventArgs^ e) {

	toolStripButton_Connect->Visible = false;
	toolStripButton_Stop->Visible = true;

	auto handle = msclr::interop::marshal_as<std::wstring>(label_HandleValue->Text);
	auto message = msclr::interop::marshal_as<std::wstring>(textBox_Message->Text);

	_dpipe = DPipeBuilder::Create(handle);

	AddPair(_dpipe, this);

	_dpuser = new DPUser(_dpipe, false);

	_dpfileTransporter = new DPFileTransporter(_dpuser, 10);
	_dpfileTransporter->SetFileReceivedCallback(HandlerFileReceive);

	_dpuser->SetOnOtherSideDisconnectCallback(OnOtherSideDisconnect);
	_dpuser->SetMessageStringReceivedHandler(OnMessageStringReceived);
	_dpuser->SetInfoStringReceivedHandler(OnInfoStringReceived);
	_dpuser->SetWarningStringReceivedHandler(OnWarningStringReceived);
	_dpuser->SetErrorStringReceivedHandler(OnErrorStringReceived);

	_dpuser->SetHandler(1, Handler1);

	try {
		_dpuser->Connect(handle, message);
		toolStripLabel_Status->Text = "Connected";
		button_Send->Enabled = true;

		if (_dpipe->Type() == DP_TYPE::ANONYMOUS_PIPE)
			comboBox_PipeType->SelectedIndex = 0;
		else
			comboBox_PipeType->SelectedIndex = 1;

		button_Request1->Enabled = true;
		button_SendFile->Enabled = true;
	}
	catch (Exception^ ex) {
		MessageBox::Show("Unable to connect the pipe: " + ex->Message, "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
	}
}
private: System::Void clearToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	listBox_Receive->Items->Clear();
}
private: System::Void toolStripButton_down_Click(System::Object^ sender, System::EventArgs^ e) {
	toolStripButton_down->Visible = false;
	toolStripButton_Up->Visible = true;

	this->Height += 180;
}
private: System::Void toolStripButton_Up_Click(System::Object^ sender, System::EventArgs^ e) {
	toolStripButton_down->Visible = true;
	toolStripButton_Up->Visible = false;

	this->Height -= 180;
}
private: System::Void contextMenuStrip_Memory_Opening(System::Object^ sender, System::ComponentModel::CancelEventArgs^ e) {

	if (_dpuser == nullptr) {
		freeToolStripMenuItem->Enabled = false;
	}
	else {
		if (dataGridView_MemoryAllocations->SelectedRows->Count > 0) {

			auto value = dataGridView_MemoryAllocations->SelectedRows[0]->Cells[0]->Value;

			if (value != nullptr) {
				selectedMemoryAddress = System::UInt64::Parse(value->ToString(), System::Globalization::NumberStyles::HexNumber);
				freeToolStripMenuItem->Enabled = true;
			}
			else
				selectedMemoryAddress = 0;
		}
	}
		
}
private: System::Void dataGridView_MemoryAllocations_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {

	if (e->Button == System::Windows::Forms::MouseButtons::Right)
	{
		auto h = dataGridView_MemoryAllocations->HitTest(e->X, e->Y);
		if (h->Type == DataGridViewHitTestType::Cell)
		{
			dataGridView_MemoryAllocations->Rows[h->RowIndex]->Selected = true;
		}
	}
}
private: System::Void freeToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	if (selectedMemoryAddress > 0 ) {
		_dpuser->FreeAllocatedMemory(selectedMemoryAddress);
	}

	RefreshDataAllocationList();
}

private: delegate void ShowDataAllocatedDelegate(System::String^ megabytes);

private: void ShowDataAllocatedFunction(System::String^ megabytes) {
	
	label_MemoryAllocatedValue->Text = megabytes + " MB";
}

private: void OnMemoryAllocation(void* address, DWORD size) {
	
	auto memoryAllocated = HeapAllocatedData::GetMemoryAllocatedMB();
	auto memoryAllocatedString = memoryAllocated.ToString();

	if (label_MemoryAllocatedValue->InvokeRequired) {
		label_MemoryAllocatedValue->BeginInvoke(gcnew ShowDataAllocatedDelegate(this, &FormMain::ShowDataAllocatedFunction), memoryAllocatedString);
	}
	else {
		label_MemoryAllocatedValue->Text = memoryAllocatedString + " MB";
	}
}

private: void OnMemoryDeallocation(void* address, DWORD size) {

	auto memoryAllocated = HeapAllocatedData::GetMemoryAllocatedMB();
	auto memoryAllocatedString = memoryAllocated.ToString();

	if (label_MemoryAllocatedValue->InvokeRequired) {
		label_MemoryAllocatedValue->BeginInvoke(gcnew ShowDataAllocatedDelegate(this, &FormMain::ShowDataAllocatedFunction), memoryAllocatedString);
	}
	else {
		label_MemoryAllocatedValue->Text = memoryAllocatedString + " MB";
	}
}

private: void RefreshDataAllocationList() {
	dataGridView_MemoryAllocations->Rows->Clear();

	std::list<std::pair<void*, DWORD>> list;
	_dpuser->ReadAllocatedAddresses(list);

	for (auto pair : list) {
		std::wstringstream address;
		address << std::hex << pair.first;
		auto size = HeapAllocatedData::GetSizeW(pair.second);
		auto newRowIndex = dataGridView_MemoryAllocations->Rows->Add();

		dataGridView_MemoryAllocations->Rows[newRowIndex]->Cells[0]->Value = gcnew System::String(address.str().c_str());
		dataGridView_MemoryAllocations->Rows[newRowIndex]->Cells[1]->Value = gcnew System::String(size.c_str());
	}
}

private: System::Void button_Refresh_Click(System::Object^ sender, System::EventArgs^ e) {

	RefreshDataAllocationList();

}
private: System::Void checkBox_KeepRequestData_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {

	bool value = (bool)checkBox_KeepRequestData->Checked;

	if (_dpuser != nullptr) {
		_dpuser->keepRequestMemoryAllocated(value);
	}
}
private: System::Void checkBox_KeepResponseData_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {

	bool value = (bool)checkBox_KeepResponseData->Checked;

	if (_dpuser != nullptr) {
		_dpuser->keepResponseMemoryAllocated(value);
	}
}
private: static void Handler1(DPReceivedRequest& req) {

	auto server = req.server();
	auto resp = req.createRespose();
	resp.code = 0;
	server->SendResponse(req, resp);
}

private: static void HandlerFileReceive(DPFileReceiverRequest& req) {

	auto fileName = gcnew System::String(req.GetFileNameW().c_str());

	auto saveFileDialog = gcnew System::Windows::Forms::SaveFileDialog();
	saveFileDialog->FileName = fileName;
	saveFileDialog->ShowHelp = true;
	saveFileDialog->InitialDirectory = System::IO::Directory::GetCurrentDirectory();
	saveFileDialog->Filter = "All files (*.*)|*.*";

	auto result = saveFileDialog->ShowDialog();

	if (result == System::Windows::Forms::DialogResult::OK) {

		auto fileNameNew = msclr::interop::marshal_as<std::wstring>(saveFileDialog->FileName);

		std::ofstream file(fileNameNew, std::ios::out | std::ios::binary);

		req.SaveFile(file, 4096);

		file.close();
	}
}
private: System::Void button_Request1_Click(System::Object^ sender, System::EventArgs^ e) {

	DWORD size;

	if (comboBox_RequestSize->Text == "2 MB")
		size = 2 * 1024 * 1024;
	else if (comboBox_RequestSize->Text == "4 MB")
		size = 4 * 1024 * 1024;
	else if (comboBox_RequestSize->Text == "8 MB")
		size = 8 * 1024 * 1024;
	else if (comboBox_RequestSize->Text == "16 MB")
		size = 16 * 1024 * 1024;
	else if (comboBox_RequestSize->Text == "32 MB")
		size = 32 * 1024 * 1024;
	else
		size = 1 * 1024 * 1024;

	std::vector<char> buffer(size);

	_dpuser->SendRequest(1, buffer.data(), size);
}

private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {

	auto openFileDialog = gcnew System::Windows::Forms::OpenFileDialog();
	openFileDialog->InitialDirectory = System::IO::Directory::GetCurrentDirectory();
	openFileDialog->ShowHelp = true;
	openFileDialog->Filter = "All files (*.*)|*.*";
	auto resuslt = openFileDialog->ShowDialog();

	if (resuslt == System::Windows::Forms::DialogResult::OK) {

		auto fileName = msclr::interop::marshal_as<std::wstring>(openFileDialog->SafeFileName);

		std::ifstream file(fileName, std::ios::binary);

		_dpfileTransporter->SendFile(file, fileName, 4096);

		file.close();
	}
}
private: System::Void freeAllToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {

	HeapAllocatedData::Free();
	RefreshDataAllocationList();

}
};
}
