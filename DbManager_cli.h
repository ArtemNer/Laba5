#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#using <System.dll>
using namespace System;
using namespace System::Collections::Generic;

// ќбЄртка C++/CLI над sqlite3 (native).
public ref class DbManager_cli
{
private:
    // скрытое поле Ч указатель на sqlite3* (native)
    void* rawHandle; // будем хранить sqlite3* как void*

    bool isOpen;

public:
    DbManager_cli(System::String^ dbFile);
    ~DbManager_cli();
    !DbManager_cli();

    property bool IsOpen {
        bool get() { return isOpen; }
    }

    bool Open();
    void Close();

    // CRUD-like
    List<array<Object^>^>^ ReadAll();
    void ClearTable();
    void Insert(System::String^ name, double basePay, double bonusPercent);
    void InsertBatch(List<array<Object^>^>^ rows);

private:
    System::String^ dbPath;
    void ThrowIfError(int rc, const char* context);
    std::string ToStdString(System::String^ s);
    System::String^ ToSystemString(const std::string& s);
};
