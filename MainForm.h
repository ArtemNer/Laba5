#pragma once

#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>

#include "DbManagerCLI.h"
#include "Payroll.h"
#include "EditForm.h"
#include <msclr/marshal_cppstd.h>


public ref class MainForm : public System::Windows::Forms::Form
{
public:
    MainForm()
    {
        InitializeLogic(nullptr, true); // по умолчанию считаем админом
    }

    // Основной конструктор — получаем имя пользователя и флаг админа
    MainForm(System::String^ userName, bool isAdminFlag)
    {
        InitializeLogic(userName, isAdminFlag);
    }

private:
    void InitializeLogic(System::String^ userName, bool isAdminFlag)
    {
        dept = new PayrollDepartment();
        isAdmin = isAdminFlag;

        // инициализация БД (файл payroll.db рядом с exe)
        System::String^ dbPath = System::IO::Path::Combine(
            System::Windows::Forms::Application::StartupPath, "payroll.db");
        db = gcnew DbManagerCLI(dbPath);


        InitializeComponent();

        // Добавим имя пользователя в заголовок
        if (!System::String::IsNullOrEmpty(userName))
        {
            this->Text = this->Text + " — " + userName;
        }

        // Загрузить данные из БД в dept и обновить сетку
        LoadFromDatabaseToDept();

        UpdateAccessRights();
    }




protected:
    ~MainForm()
    {
        if (dept) delete dept;
        db = nullptr;
    }

private:
    PayrollDepartment* dept;
    DbManagerCLI^ db;    // <-- managed wrapper (DbManagerCLI), а не DbManager
    bool isAdmin; // <--- добавили


    System::Windows::Forms::DataGridView^ dgv;
    System::Windows::Forms::Button^ btnAdd;
    System::Windows::Forms::Button^ btnEdit;
    System::Windows::Forms::Button^ btnDelete;
    System::Windows::Forms::Button^ btnLoad;
    System::Windows::Forms::Button^ btnSave;
    System::Windows::Forms::Button^ btnSortName;
    System::Windows::Forms::Button^ btnSortPay;
    System::Windows::Forms::Button^ btnAverage;
    System::Windows::Forms::Label^ lblAverage;

    void UpdateAccessRights()
    {
        if (!isAdmin)
        {
            // Обычному пользователю запрещаем менять данные
            btnAdd->Enabled = false;
            btnEdit->Enabled = false;
            btnDelete->Enabled = false;
            btnSave->Enabled = false;

            // Остальное (просмотр/загрузка/сортировка/средняя) оставляем
            btnLoad->Enabled = true;
            btnSortName->Enabled = true;
            btnSortPay->Enabled = true;
            btnAverage->Enabled = true;
        }
        else
        {
            // Админу всё разрешено
            btnAdd->Enabled = true;
            btnEdit->Enabled = true;
            btnDelete->Enabled = true;
            btnSave->Enabled = true;
        }
    }


    // === Инициализация главной формы ===
    void InitializeComponent()
    {
        this->Text = "Отдел расчёта зарплаты";
        this->ClientSize = System::Drawing::Size(900, 600);
        this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;

        int x = 10;
        int y = 10;
        int w = 120;
        int h = 30;
        int gap = 5;

        // Кнопка "Добавить"
        btnAdd = gcnew System::Windows::Forms::Button();
        btnAdd->Text = "Добавить";
        btnAdd->Location = System::Drawing::Point(x, y);
        btnAdd->Size = System::Drawing::Size(w, h);
        btnAdd->Click += gcnew System::EventHandler(this, &MainForm::OnAddClick);
        this->Controls->Add(btnAdd);

        // Кнопка "Изменить"
        btnEdit = gcnew System::Windows::Forms::Button();
        btnEdit->Text = "Изменить";
        btnEdit->Location = System::Drawing::Point(x + (w + gap), y);
        btnEdit->Size = System::Drawing::Size(w, h);
        btnEdit->Click += gcnew System::EventHandler(this, &MainForm::OnEditClick);
        this->Controls->Add(btnEdit);

        // Кнопка "Удалить"
        btnDelete = gcnew System::Windows::Forms::Button();
        btnDelete->Text = "Удалить";
        btnDelete->Location = System::Drawing::Point(x + 2 * (w + gap), y);
        btnDelete->Size = System::Drawing::Size(w, h);
        btnDelete->Click += gcnew System::EventHandler(this, &MainForm::OnDeleteClick);
        this->Controls->Add(btnDelete);

        // Кнопка "Открыть файл"
        btnLoad = gcnew System::Windows::Forms::Button();
        btnLoad->Text = "Открыть файл";
        btnLoad->Location = System::Drawing::Point(x + 3 * (w + gap), y);
        btnLoad->Size = System::Drawing::Size(w, h);
        btnLoad->Click += gcnew System::EventHandler(this, &MainForm::OnLoadClick);
        this->Controls->Add(btnLoad);

        // Кнопка "Сохранить файл"
        btnSave = gcnew System::Windows::Forms::Button();
        btnSave->Text = "Сохранить файл";
        btnSave->Location = System::Drawing::Point(x + 4 * (w + gap), y);
        btnSave->Size = System::Drawing::Size(w, h);
        btnSave->Click += gcnew System::EventHandler(this, &MainForm::OnSaveClick);
        this->Controls->Add(btnSave);

        // Кнопка "Сорт. по названию"
        btnSortName = gcnew System::Windows::Forms::Button();
        btnSortName->Text = "Сорт. по названию";
        btnSortName->Location = System::Drawing::Point(x + 5 * (w + gap), y);
        btnSortName->Size = System::Drawing::Size(w + 40, h);
        btnSortName->Click += gcnew System::EventHandler(this, &MainForm::OnSortNameClick);
        this->Controls->Add(btnSortName);

        // Кнопка "Сорт. по оплате"
        btnSortPay = gcnew System::Windows::Forms::Button();
        btnSortPay->Text = "Сорт. по оплате";
        btnSortPay->Location = System::Drawing::Point(x + w + 250, y + h + gap);
        btnSortPay->Size = System::Drawing::Size(w, h);
        btnSortPay->Click += gcnew System::EventHandler(this, &MainForm::OnSortPayClick);
        this->Controls->Add(btnSortPay);

        // Кнопка "Средняя оплата"
        btnAverage = gcnew System::Windows::Forms::Button();
        btnAverage->Text = "Средняя оплата";
        btnAverage->Location = System::Drawing::Point(x, y + h + gap);
        btnAverage->Size = System::Drawing::Size(w + 40, h);
        btnAverage->Click += gcnew System::EventHandler(this, &MainForm::OnAverageClick);
        this->Controls->Add(btnAverage);

        // Метка с результатом средней оплаты
        lblAverage = gcnew System::Windows::Forms::Label();
        lblAverage->AutoSize = true;
        lblAverage->Location = System::Drawing::Point(x + w + 60, y + h + gap + 8);
        this->Controls->Add(lblAverage);

        // Таблица
        dgv = gcnew System::Windows::Forms::DataGridView();
        dgv->Location = System::Drawing::Point(10, 80);
        dgv->Size = System::Drawing::Size(860, 480);
        dgv->AllowUserToAddRows = false;
        dgv->ReadOnly = true;
        dgv->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
        dgv->MultiSelect = false;
        dgv->AutoSizeColumnsMode = System::Windows::Forms::DataGridViewAutoSizeColumnsMode::Fill;

        dgv->Columns->Add("Name", "Тип работ");
        dgv->Columns->Add("BasePay", "Базовая оплата");
        dgv->Columns->Add("Bonus", "Надбавка, %");
        dgv->Columns->Add("FinalPay", "Итоговая оплата");

        this->Controls->Add(dgv);
    }

    // ===== Вспомогательные методы =====

    System::String^ ToSystemString(const std::string& s)
    {
        return gcnew System::String(s.c_str());
    }

    std::string ToStdString(System::String^ s)
    {
        return msclr::interop::marshal_as<std::string>(s);
    }

    void ShowError(System::String^ msg)
    {
        System::Windows::Forms::MessageBox::Show(
            msg,
            "Ошибка",
            System::Windows::Forms::MessageBoxButtons::OK,
            System::Windows::Forms::MessageBoxIcon::Error);
    }

    int GetSelectedIndex()
    {
        if (dgv == nullptr) return -1;
        if (dgv->SelectedRows->Count == 0) return -1;
        return dgv->SelectedRows[0]->Index;
    }


    void RefreshGrid()
    {
        dgv->Rows->Clear();

        const std::vector<std::shared_ptr<IWorkType>>& items =
            dept->getWorkTypes();

        for (int i = 0; i < static_cast<int>(items.size()); ++i)
        {
            std::shared_ptr<IWorkType> w = items[i];
            int row = dgv->Rows->Add();
            dgv->Rows[row]->Cells["Name"]->Value =
                ToSystemString(w->getName());
            dgv->Rows[row]->Cells["BasePay"]->Value =
                w->getBasePay();
            dgv->Rows[row]->Cells["Bonus"]->Value =
                w->getBonusPercent();
            dgv->Rows[row]->Cells["FinalPay"]->Value =
                w->getFinalPay();
        }

        lblAverage->Text = "";
    }

    void LoadFromDatabaseToDept()
    {
        try
        {
            dept->clear();
            System::Collections::Generic::List<System::Tuple<System::String^, double, double>^>^ rows = db->GetAll();
            for each (System::Tuple<System::String^, double, double> ^ t in rows)
            {
                std::string name = ToStdString(t->Item1);
                double basePay = t->Item2;
                double bonus = t->Item3;
                // Using dept interface — может бросить исключение, оборачиваем
                dept->addWorkType(name, basePay, bonus);
            }
            RefreshGrid();
        }
        catch (const PayrollException& ex) { ShowError(ToSystemString(ex.what())); }
        catch (System::Exception^ ex) { ShowError(ex->Message); }
        catch (const std::exception& ex) { ShowError(ToSystemString(ex.what())); }
    }

    void SaveDeptToDatabase()
    {
        try
        {
            db->ClearTable();
            const std::vector<std::shared_ptr<IWorkType>>& items = dept->getWorkTypes();
            for (size_t i = 0; i < items.size(); ++i)
            {
                std::shared_ptr<IWorkType> w = items[i];
                db->Insert(ToSystemString(w->getName()), w->getBasePay(), w->getBonusPercent());
            }
        }
        catch (System::Exception^ ex) { ShowError(ex->Message); }
        catch (const std::exception& ex) { ShowError(ToSystemString(ex.what())); }
    }


    // ===== Обработчики кнопок =====

    void OnAddClick(System::Object^ sender, System::EventArgs^ e)
    {
        EditForm^ dlg = gcnew EditForm();
        if (dlg->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
        {
            try {
                std::string name = ToStdString(dlg->WorkName);
                dept->addWorkType(name, dlg->BasePay, dlg->BonusPercent);
                SaveDeptToDatabase();
                RefreshGrid();
            }
            catch (const PayrollException& ex) {
                ShowError(ToSystemString(ex.what()));
            }
            catch (const std::exception& ex) {
                ShowError(ToSystemString(ex.what()));
            }
        }
    }

    void OnEditClick(System::Object^ sender, System::EventArgs^ e)
    {
        int idx = GetSelectedIndex();
        if (idx < 0) {
            System::Windows::Forms::MessageBox::Show(
                "Выберите строку для редактирования",
                "Внимание",
                System::Windows::Forms::MessageBoxButtons::OK,
                System::Windows::Forms::MessageBoxIcon::Warning);
            return;
        }

        const std::vector<std::shared_ptr<IWorkType>>& items =
            dept->getWorkTypes();
        std::shared_ptr<IWorkType> w = items[idx];

        EditForm^ dlg = gcnew EditForm(
            ToSystemString(w->getName()),
            w->getBasePay(),
            w->getBonusPercent(),
            true // имя не меняем
        );

        if (dlg->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
        {
            try {
                std::string name = ToStdString(dlg->WorkName);
                dept->updateWorkType(idx, name, dlg->BasePay, dlg->BonusPercent);
                SaveDeptToDatabase();
                RefreshGrid();
            }
            catch (const PayrollException& ex) {
                ShowError(ToSystemString(ex.what()));
            }
            catch (const std::exception& ex) {
                ShowError(ToSystemString(ex.what()));
            }
        }
    }

    void OnDeleteClick(System::Object^ sender, System::EventArgs^ e)
    {
        int idx = GetSelectedIndex();
        if (idx < 0) {
            System::Windows::Forms::MessageBox::Show(
                "Выберите строку для удаления",
                "Внимание",
                System::Windows::Forms::MessageBoxButtons::OK,
                System::Windows::Forms::MessageBoxIcon::Warning);
            return;
        }

        if (System::Windows::Forms::MessageBox::Show(
            "Удалить выбранный тип работ?",
            "Подтверждение",
            System::Windows::Forms::MessageBoxButtons::YesNo,
            System::Windows::Forms::MessageBoxIcon::Question)
            != System::Windows::Forms::DialogResult::Yes)
            return;

        try {
            dept->removeWorkType(idx);
            SaveDeptToDatabase();
            RefreshGrid();
        }
        catch (const PayrollException& ex) {
            ShowError(ToSystemString(ex.what()));
        }
    }

    void OnLoadClick(System::Object^ sender, System::EventArgs^ e)
    {
        System::Windows::Forms::OpenFileDialog^ dlg = gcnew System::Windows::Forms::OpenFileDialog();
        dlg->Filter = "CSV/TXT (*.csv;*.txt)|*.csv;*.txt|Все файлы (*.*)|*.*";

        if (dlg->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
        {
            try {
                // Спросим — заменить таблицу или добавить
                auto res = System::Windows::Forms::MessageBox::Show(
                    "Импортировать файл. Заменить текущие данные в базе? (Yes = заменить, No = добавить)",
                    "Импорт",
                    System::Windows::Forms::MessageBoxButtons::YesNoCancel,
                    System::Windows::Forms::MessageBoxIcon::Question);

                if (res == System::Windows::Forms::DialogResult::Cancel) return;

                if (res == System::Windows::Forms::DialogResult::Yes) {
                    db->ClearTable(); // очистим БД перед импортом
                }

                db->ImportFromFile(dlg->FileName);   // импорт в БД
                LoadFromDatabaseToDept();            // обновляем рабочие данные приложения

                System::Windows::Forms::MessageBox::Show("Импорт завершён.", "Готово",
                    System::Windows::Forms::MessageBoxButtons::OK,
                    System::Windows::Forms::MessageBoxIcon::Information);
            }
            catch (System::Exception^ ex) {
                ShowError(ex->Message);
            }
        }
    }


    void OnSaveClick(System::Object^ sender, System::EventArgs^ e)
    {
        System::Windows::Forms::SaveFileDialog^ dlg = gcnew System::Windows::Forms::SaveFileDialog();
        dlg->Filter = "CSV (comma separated) (*.csv)|*.csv|Текстовые файлы (*.txt)|*.txt|Все файлы (*.*)|*.*";
        dlg->DefaultExt = "csv";
        dlg->AddExtension = true;
        dlg->OverwritePrompt = true;

        if (dlg->ShowDialog(this) == System::Windows::Forms::DialogResult::OK)
        {
            try {
                db->ExportToFile(dlg->FileName);
                System::Windows::Forms::MessageBox::Show("Экспорт завершён.", "Готово",
                    System::Windows::Forms::MessageBoxButtons::OK,
                    System::Windows::Forms::MessageBoxIcon::Information);
            }
            catch (System::Exception^ ex) {
                ShowError(ex->Message);
            }
        }
    }


    void OnSortNameClick(System::Object^ sender, System::EventArgs^ e)
    {
        dept->sortByName(true);
        RefreshGrid();
    }

    void OnSortPayClick(System::Object^ sender, System::EventArgs^ e)
    {
        dept->sortByFinalPay(false);
        RefreshGrid();
    }

    void OnAverageClick(System::Object^ sender, System::EventArgs^ e)
    {
        try {
            double avg = dept->calculateAveragePay();
            System::String^ s = avg.ToString("F2");
            lblAverage->Text = "Средняя оплата: " + s;
            System::Windows::Forms::MessageBox::Show(
                "Средняя оплата: " + s,
                "Средняя оплата",
                System::Windows::Forms::MessageBoxButtons::OK,
                System::Windows::Forms::MessageBoxIcon::Information);
        }
        catch (const PayrollException& ex) {
            ShowError(ToSystemString(ex.what()));
        }
    }
};
