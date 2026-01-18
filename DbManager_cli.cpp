#include "DbManager_cli.h"
#include "sqlite3.h" // из добавленных файлов sqlite3.c/.h
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

DbManager_cli::DbManager_cli(System::String^ dbFile)
{
    rawHandle = nullptr;
    isOpen = false;
    dbPath = dbFile;
}

DbManager_cli::~DbManager_cli()
{
    this->!DbManager_cli();
}

DbManager_cli::!DbManager_cli()
{
    if (rawHandle != nullptr) {
        sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);
        sqlite3_close(db);
        rawHandle = nullptr;
        isOpen = false;
    }
}

std::string DbManager_cli::ToStdString(System::String^ s)
{
    return marshal_as<std::string>(s);
}

System::String^ DbManager_cli::ToSystemString(const std::string& s)
{
    return gcnew System::String(s.c_str());
}

void DbManager_cli::ThrowIfError(int rc, const char* context)
{
    if (rc != SQLITE_OK && rc != SQLITE_DONE && rc != SQLITE_ROW) {
        const char* err = nullptr;
        sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);
        if (db) err = sqlite3_errmsg(db);
        System::String^ em = "SQLite error";
        if (err) em = ToSystemString(std::string(err));
        throw gcnew System::Exception(ToSystemString(std::string(context) + ": " + (err ? err : "unknown")));
    }
}

bool DbManager_cli::Open()
{
    if (isOpen) return true;
    sqlite3* db = nullptr;
    std::string path = ToStdString(dbPath);
    int rc = sqlite3_open(path.c_str(), &db);
    if (rc != SQLITE_OK) {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
        return false;
    }
    rawHandle = reinterpret_cast<void*>(db);
    isOpen = true;

    // Создадим таблицу, если её нет
    const char* sql = "CREATE TABLE IF NOT EXISTS WorkTypes ("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT NOT NULL UNIQUE, "
        "BasePay REAL NOT NULL, "
        "BonusPercent REAL NOT NULL);";
    char* errmsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        sqlite3_close(db);
        rawHandle = nullptr;
        isOpen = false;
        return false;
    }

    return true;
}

void DbManager_cli::Close()
{
    if (!isOpen) return;
    sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);
    if (db) {
        sqlite3_close(db);
        rawHandle = nullptr;
        isOpen = false;
    }
}

List<array<Object^>^>^ DbManager_cli::ReadAll()
{
    List<array<Object^>^>^ res = gcnew List<array<Object^>^>();
    if (!isOpen) return res;

    sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);
    const char* sql = "SELECT Name, BasePay, BonusPercent FROM WorkTypes ORDER BY Id;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        ThrowIfError(rc, "prepare ReadAll");
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        double basePay = sqlite3_column_double(stmt, 1);
        double bonus = sqlite3_column_double(stmt, 2);

        System::String^ name = ToSystemString(text ? reinterpret_cast<const char*>(text) : "");
        array<Object^>^ row = gcnew array<Object^>(3);
        row[0] = name;
        row[1] = basePay;
        row[2] = bonus;
        res->Add(row);
    }

    if (stmt) sqlite3_finalize(stmt);

    return res;
}

void DbManager_cli::ClearTable()
{
    if (!isOpen) return;
    sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);
    const char* sql = "DELETE FROM WorkTypes;";
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        ThrowIfError(rc, "ClearTable");
    }
}

void DbManager_cli::Insert(System::String^ name, double basePay, double bonusPercent)
{
    if (!isOpen) return;
    sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);

    const char* sql = "INSERT OR REPLACE INTO WorkTypes (Name, BasePay, BonusPercent) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        ThrowIfError(rc, "prepare Insert");
        return;
    }

    std::string sname = ToStdString(name);
    sqlite3_bind_text(stmt, 1, sname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, basePay);
    sqlite3_bind_double(stmt, 3, bonusPercent);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        ThrowIfError(rc, "execute Insert");
    }

    sqlite3_finalize(stmt);
}

void DbManager_cli::InsertBatch(List<array<Object^>^>^ rows)
{
    if (!isOpen) return;
    sqlite3* db = reinterpret_cast<sqlite3*>(rawHandle);

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        ThrowIfError(rc, "begin transaction");
    }

    try {
        for each (array<Object^> ^ r in rows) {
            System::String^ name = safe_cast<System::String^>(r[0]);
            double basePay = safe_cast<double>(r[1]);
            double bonus = safe_cast<double>(r[2]);
            Insert(name, basePay, bonus);
        }
        rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errmsg);
        if (rc != SQLITE_OK) {
            if (errmsg) sqlite3_free(errmsg);
            ThrowIfError(rc, "commit transaction");
        }
    }
    catch (...) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errmsg);
        throw;
    }
}
