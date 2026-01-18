#pragma once

#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>

public ref class LoginForm : public System::Windows::Forms::Form
{
public:
    // Обычный режим "Вход"
    LoginForm()
    {
        InitializeComponent();
        this->Text = "Вход в систему";
        labelTitle->Text = "Вход в систему";
    }

    // Перегрузка конструктора – можем поменять заголовок (например "Регистрация")
    LoginForm(System::String^ customTitle)
    {
        InitializeComponent();
        this->Text = customTitle;
        labelTitle->Text = customTitle;
    }

    // Данные, которые заберёт Program.cpp
    property System::String^ UserName;
    property bool IsAdmin;

protected:
    ~LoginForm()
    {
        if (components != nullptr)
            delete components;
    }

private:
    System::Windows::Forms::Label^ labelTitle;
    System::Windows::Forms::Label^ labelUser;
    System::Windows::Forms::Label^ labelPass;
    System::Windows::Forms::TextBox^ textUser;
    System::Windows::Forms::TextBox^ textPass;
    System::Windows::Forms::CheckBox^ checkAdmin;
    System::Windows::Forms::Button^ btnOK;
    System::Windows::Forms::Button^ btnCancel;
    System::Windows::Forms::ErrorProvider^ errorProvider;
    System::ComponentModel::IContainer^ components;

    void InitializeComponent()
    {
        components = gcnew System::ComponentModel::Container();

        this->Width = 380;
        this->Height = 230;
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
        this->MaximizeBox = false;
        this->MinimizeBox = false;
        this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;

        // Заголовок
        labelTitle = gcnew System::Windows::Forms::Label();
        labelTitle->AutoSize = true;
        labelTitle->Location = System::Drawing::Point(10, 10);
        labelTitle->Font = gcnew System::Drawing::Font(
            "Segoe UI", 10, System::Drawing::FontStyle::Bold);
        this->Controls->Add(labelTitle);

        // "Имя пользователя"
        labelUser = gcnew System::Windows::Forms::Label();
        labelUser->Text = "Имя пользователя:";
        labelUser->Location = System::Drawing::Point(10, 50);
        labelUser->AutoSize = true;
        this->Controls->Add(labelUser);

        textUser = gcnew System::Windows::Forms::TextBox();
        textUser->Location = System::Drawing::Point(150, 47);
        textUser->Width = 200;
        this->Controls->Add(textUser);

        // Чекбокс "Администратор"
        checkAdmin = gcnew System::Windows::Forms::CheckBox();
        checkAdmin->Text = "Я администратор отдела";
        checkAdmin->Location = System::Drawing::Point(13, 80);
        checkAdmin->AutoSize = true;
        checkAdmin->CheckedChanged += gcnew System::EventHandler(this, &LoginForm::OnAdminCheckedChanged);
        this->Controls->Add(checkAdmin);

        // "Пароль"
        labelPass = gcnew System::Windows::Forms::Label();
        labelPass->Text = "Пароль администратора:";
        labelPass->Location = System::Drawing::Point(10, 110);
        labelPass->AutoSize = true;
        this->Controls->Add(labelPass);

        textPass = gcnew System::Windows::Forms::TextBox();
        textPass->Location = System::Drawing::Point(180, 107);
        textPass->Width = 170;
        textPass->PasswordChar = '*';
        this->Controls->Add(textPass);

        // Кнопка OK
        btnOK = gcnew System::Windows::Forms::Button();
        btnOK->Text = "OK";
        btnOK->Location = System::Drawing::Point(100, 150);
        btnOK->Click += gcnew System::EventHandler(this, &LoginForm::OnOkClick);
        this->Controls->Add(btnOK);

        // Кнопка Cancel
        btnCancel = gcnew System::Windows::Forms::Button();
        btnCancel->Text = "Отмена";
        btnCancel->Location = System::Drawing::Point(200, 150);
        btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
        this->Controls->Add(btnCancel);

        this->AcceptButton = btnOK;
        this->CancelButton = btnCancel;

        // ErrorProvider
        errorProvider = gcnew System::Windows::Forms::ErrorProvider();
        errorProvider->ContainerControl = this;

        // По умолчанию поля пароля выключены, пока не выбрана галка "Администратор"
        SetAdminControlsEnabled(false);
    }

    void SetAdminControlsEnabled(bool enabled)
    {
        labelPass->Enabled = enabled;
        textPass->Enabled = enabled;
        if (!enabled)
        {
            textPass->Text = "";
            errorProvider->SetError(textPass, "");
        }
    }

    void OnAdminCheckedChanged(System::Object^ sender, System::EventArgs^ e)
    {
        SetAdminControlsEnabled(checkAdmin->Checked);
    }

    bool ValidateAll()
    {
        errorProvider->Clear();
        bool ok = true;

        System::String^ name = textUser->Text->Trim();
        if (System::String::IsNullOrEmpty(name))
        {
            errorProvider->SetError(textUser, "Имя не может быть пустым");
            ok = false;
        }
        else if (name->Length > 30)
        {
            errorProvider->SetError(textUser, "Слишком длинное имя (макс. 30 символов)");
            ok = false;
        }

        // Если выбран админ — проверяем И имя, и пароль
        if (checkAdmin->Checked)
        {
            System::String^ pass = textPass->Text;
            System::String^ correctPass = "admin123";

            // 1) Имя должно быть строго "Admin"
            if (name != "Admin")
            {
                errorProvider->SetError(textUser,
                    "Неверное имя администратора");
                ok = false;
            }

            // 2) Проверка пароля
            if (System::String::IsNullOrEmpty(pass))
            {
                errorProvider->SetError(textPass, "Введите пароль администратора");
                ok = false;
            }
            else if (pass != correctPass)
            {
                errorProvider->SetError(textPass, "Неверный пароль");
                ok = false;
            }
        }

        return ok;
    }


    void OnOkClick(System::Object^ sender, System::EventArgs^ e)
    {
        if (!ValidateAll()) return;

        UserName = textUser->Text->Trim();
        IsAdmin = checkAdmin->Checked;

        this->DialogResult = System::Windows::Forms::DialogResult::OK;
        this->Close();
    }
};
