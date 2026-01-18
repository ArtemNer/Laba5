#include "NativeDb.h"
#include "sqlite3.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

NativeDb::NativeDb(const std::string& dbPath)
    : path(dbPath), dbHandle(nullptr)
{
    openDb();
    initialize();
}

NativeDb::~NativeDb()
{
    closeDb();
}

void NativeDb::openDb()
{
    int rc = sqlite3_open(path.c_str(), &dbHandle);
    if (rc != SQLITE_OK) {
        std::string msg = "Cannot open DB: ";
        msg += sqlite3_errmsg(dbHandle);
        sqlite3_close(dbHandle);
        dbHandle = nullptr;
        throw std::runtime_error(msg);
    }
    sqlite3_exec(dbHandle, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
}

void NativeDb::closeDb()
{
    if (dbHandle) {
        sqlite3_close(dbHandle);
        dbHandle = nullptr;
    }
}

void NativeDb::initialize()
{
    const char* sql =
        "CREATE TABLE IF NOT EXISTS WorkTypes ("
        "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Name TEXT UNIQUE NOT NULL, "
        "BasePay REAL NOT NULL, "
        "BonusPercent REAL NOT NULL"
        ");";
    char* err = nullptr;
    int rc = sqlite3_exec(dbHandle, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = "SQL error: ";
        if (err) { msg += err; sqlite3_free(err); }
        throw std::runtime_error(msg);
    }
}

std::vector<std::tuple<std::string, double, double>> NativeDb::getAll()
{
    std::vector<std::tuple<std::string, double, double>> out;
    const char* sql = "SELECT Name, BasePay, BonusPercent FROM WorkTypes ORDER BY Name;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("prepare failed");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        double base = sqlite3_column_double(stmt, 1);
        double bonus = sqlite3_column_double(stmt, 2);
        std::string name = text ? reinterpret_cast<const char*>(text) : std::string();
        out.emplace_back(name, base, bonus);
    }
    sqlite3_finalize(stmt);
    return out;
}

void NativeDb::clearTable()
{
    const char* sql = "DELETE FROM WorkTypes;";
    char* err = nullptr;
    int rc = sqlite3_exec(dbHandle, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = "SQL error: ";
        if (err) { msg += err; sqlite3_free(err); }
        throw std::runtime_error(msg);
    }
}

void NativeDb::insertOrReplace(const std::string& name, double basePay, double bonusPercent)
{
    const char* sql = "INSERT OR REPLACE INTO WorkTypes (Name, BasePay, BonusPercent) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(dbHandle, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("prepare failed");

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, basePay);
    sqlite3_bind_double(stmt, 3, bonusPercent);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("insert failed");
    }
    sqlite3_finalize(stmt);
}

static std::string trim_str(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

void NativeDb::importFromFile(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("cannot open file");

    sqlite3_exec(dbHandle, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    try {
        std::string line;
        while (std::getline(in, line)) {
            line = trim_str(line);
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string name, baseStr, bonusStr;
            if (!std::getline(iss, name, ';') ||
                !std::getline(iss, baseStr, ';') ||
                !std::getline(iss, bonusStr))
            {
                continue;
            }
            name = trim_str(name);
            double base = std::stod(trim_str(baseStr));
            double bonus = std::stod(trim_str(bonusStr));
            insertOrReplace(name, base, bonus);
        }
        sqlite3_exec(dbHandle, "COMMIT;", nullptr, nullptr, nullptr);
    }
    catch (...) {
        sqlite3_exec(dbHandle, "ROLLBACK;", nullptr, nullptr, nullptr);
        throw;
    }
}

void NativeDb::exportToFile(const std::string& filename)
{
    auto rows = getAll();
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("cannot open file for write");

    // UTF-8 BOM чтобы Excel корректно открыл русский текст
    const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
    out.write(reinterpret_cast<const char*>(bom), 3);

    for (auto& t : rows) {
        out << "\"" << std::get<0>(t) << "\","
            << std::get<1>(t) << ","
            << std::get<2>(t) << "\n";
    }
}

