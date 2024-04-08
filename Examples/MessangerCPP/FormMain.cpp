#include "FormMain.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args) {
	Application::SetCompatibleTextRenderingDefault(false);
	Application::EnableVisualStyles();
	MessangerCPP::FormMain formMain;
	Application::Run(% formMain);
	return 0;
}