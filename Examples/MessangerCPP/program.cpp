#include "FormMain.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
int main(array<String^>^ args) {

	array<String^>^ argsCmd = Environment::GetCommandLineArgs();

	Application::SetCompatibleTextRenderingDefault(false);
	Application::EnableVisualStyles();
	MessangerCPP::FormMain formMain(argsCmd);
	Application::Run(% formMain);
	return 0;
}