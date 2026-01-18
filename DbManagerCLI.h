#pragma once

#using <System.dll>
#using <System.Core.dll>

#include "NativeDb.h"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace msclr::interop;

public ref class DbManagerCLI
{
private:
    NativeDb* native;

public:
    DbManagerCLI(String^ dbPath)
    {
        std::string p = marshal_as<std::string>(dbPath);
        native = new NativeDb(p);
    }

    ~DbManagerCLI()
    {
        this->!DbManagerCLI();
    }

    !DbManagerCLI()
    {
        if (native) { delete native; native = nullptr; }
    }

    List<Tuple<String^, double, double>^>^ GetAll()
    {
        auto vec = native->getAll();
        List<Tuple<String^, double, double>^>^ list = gcnew List<Tuple<String^, double, double>^>();
        for (auto& t : vec) {
            String^ name = marshal_as<String^>(std::get<0>(t));
            double basePay = std::get<1>(t);
            double bonus = std::get<2>(t);
            list->Add(Tuple::Create(name, basePay, bonus));
        }
        return list;
    }

    void ClearTable()
    {
        native->clearTable();
    }

    void Insert(String^ name, double basePay, double bonusPercent)
    {
        native->insertOrReplace(marshal_as<std::string>(name), basePay, bonusPercent);
    }

    void ImportFromFile(String^ filename)
    {
        native->importFromFile(marshal_as<std::string>(filename));
    }

    void ExportToFile(String^ filename)
    {
        native->exportToFile(marshal_as<std::string>(filename));
    }
};
