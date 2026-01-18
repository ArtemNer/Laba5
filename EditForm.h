#pragma once

#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>

public ref class EditForm : public System::Windows::Forms::Form
{
public:
    // Режим "добавление"
    EditForm()
    {
        InitializeComponent();
        this->Text = "Добавить тип работ";
        labelTitle->Text = "Добавление типа работ";
    }

    // Режим "редактирование"
    EditForm(System::String^ name, double basePay, double bonusPercent, bool lockName)
    {
        InitializeComponent();
        this->Text = "Изменить тип работ";
        labelTitle->Text = "Редактирование типа работ";

        textName->Text = name;
        textBase->Text = basePay.ToString("F2");
        textBonus->Text = bonusPercent.ToString("F2");
        textName->ReadOnly = lockName;
    }

    // Свойства, через которые MainForm заберёт данные
    property System::String^ WorkName;
    property double BasePay;
    property double BonusPercent;

protected:
    ~EditForm()
    {
        if (components != nullptr)
            delete components;
    }

private:
    System::Windows::Forms::Label^ labelTitle;
    System::Windows::Forms::TextBox^ textName;
    System::Windows::Forms::TextBox^ textBase;
    System::Windows::Forms::TextBox^ textBonus;
    System::Windows::Forms::Button^ btnOK;
    System::Windows::Forms::Button^ btnCancel;
    System::Windows::Forms::ErrorProvider^ errorProvider;
    System::ComponentModel::IContainer^ components;

    // === Инициализация формы ===
    void InitializeComponent()
    {
        components = gcnew System::ComponentModel::Container();

        this->Width = 400;
        this->Height = 230;
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
        this->MaximizeBox = false;
        this->MinimizeBox = false;
        this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;

        // Заголовок
        labelTitle = gcnew System::Windows::Forms::Label();
        labelTitle->AutoSize = true;
        labelTitle->Location = System::Drawing::Point(10, 10);
        labelTitle->Font = gcnew System::Drawing::Font(
            "Segoe UI", 10, System::Drawing::FontStyle::Bold);
        this->Controls->Add(labelTitle);

        // "Название"
        System::Windows::Forms::Label^ lbl1 = gcnew System::Windows::Forms::Label();
        lbl1->Text = "Название:";
        lbl1->Location = System::Drawing::Point(10, 50);
        lbl1->AutoSize = true;
        this->Controls->Add(lbl1);

        textName = gcnew System::Windows::Forms::TextBox();
        textName->Location = System::Drawing::Point(120, 47);
        textName->Width = 240;
        this->Controls->Add(textName);

        // "Базовая оплата"
        System::Windows::Forms::Label^ lbl2 = gcnew System::Windows::Forms::Label();
        lbl2->Text = "Базовая оплата:";
        lbl2->Location = System::Drawing::Point(10, 80);
        lbl2->AutoSize = true;
        this->Controls->Add(lbl2);

        textBase = gcnew System::Windows::Forms::TextBox();
        textBase->Location = System::Drawing::Point(120, 77);
        textBase->Width = 120;
        this->Controls->Add(textBase);

        // "Надбавка, %"
        System::Windows::Forms::Label^ lbl3 = gcnew System::Windows::Forms::Label();
        lbl3->Text = "Надбавка, %:";
        lbl3->Location = System::Drawing::Point(10, 110);
        lbl3->AutoSize = true;
        this->Controls->Add(lbl3);

        textBonus = gcnew System::Windows::Forms::TextBox();
        textBonus->Location = System::Drawing::Point(120, 107);
        textBonus->Width = 120;
        textBonus->Text = "0";
        this->Controls->Add(textBonus);

        // Кнопка OK
        btnOK = gcnew System::Windows::Forms::Button();
        btnOK->Text = "OK";
        btnOK->Location = System::Drawing::Point(120, 150);
        btnOK->Click += gcnew System::EventHandler(this, &EditForm::OnOkClick);
        this->Controls->Add(btnOK);

        // Кнопка Cancel
        btnCancel = gcnew System::Windows::Forms::Button();
        btnCancel->Text = "Отмена";
        btnCancel->Location = System::Drawing::Point(220, 150);
        btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
        this->Controls->Add(btnCancel);

        this->AcceptButton = btnOK;
        this->CancelButton = btnCancel;

        // ErrorProvider
        errorProvider = gcnew System::Windows::Forms::ErrorProvider();
        errorProvider->ContainerControl = this;
    }

    // === Проверка введённых данных ===
    bool ValidateAll()
    {
        bool ok = true;
        errorProvider->Clear();

        // Имя
        System::String^ name = textName->Text->Trim();
        if (System::String::IsNullOrEmpty(name))
        {
            errorProvider->SetError(textName, "Название не может быть пустым");
            ok = false;
        }
        else if (name->Length > 50)
        {
            errorProvider->SetError(textName, "Максимум 50 символов");
            ok = false;
        }

        // Базовая оплата
        double basePay;
        if (!System::Double::TryParse(textBase->Text, basePay) ||
            basePay <= 0.0 || basePay > 1000000.0)
        {
            errorProvider->SetError(textBase,
                "Число > 0 и <= 1 000 000");
            ok = false;
        }

        // Надбавка
        double bonus;
        if (!System::Double::TryParse(textBonus->Text, bonus) ||
            bonus < 0.0 || bonus > 1000.0)
        {
            errorProvider->SetError(textBonus,
                "0 <= надбавка <= 1000");
            ok = false;
        }

        return ok;
    }


    // === Обработка нажатия OK ===
    void OnOkClick(System::Object^ sender, System::EventArgs^ e)
    {
        if (!ValidateAll()) return;

        WorkName = textName->Text->Trim();
        BasePay = System::Double::Parse(textBase->Text);
        BonusPercent = System::Double::Parse(textBonus->Text);

        this->DialogResult = System::Windows::Forms::DialogResult::OK;
        this->Close();
    }
};
