#include <windows.h>

#using <System.dll>
#using <System.Windows.Forms.dll>

#include "MainForm.h"
#include "LoginForm.h"

// Точка входа для Windows-приложения
[System::STAThreadAttribute]
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nShowCmd)
{
    System::Windows::Forms::Application::EnableVisualStyles();
    System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);

    // Сначала показываем форму входа
    LoginForm^ login = gcnew LoginForm(); // или gcnew LoginForm("Авторизация");
    if (login->ShowDialog() == System::Windows::Forms::DialogResult::OK)
    {
        // Если логин успешен — запускаем основную форму
        System::Windows::Forms::Application::Run(
            gcnew MainForm(login->UserName, login->IsAdmin)
        );
    }
    // Если нажали Отмена — просто выходим
    return 0;
}
