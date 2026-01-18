#pragma once
#include <string>
#include <vector>
#include <tuple>

class NativeDb {
public:
    explicit NativeDb(const std::string& dbPath);
    ~NativeDb();

    void initialize();

    std::vector<std::tuple<std::string, double, double>> getAll();
    void clearTable();
    void insertOrReplace(const std::string& name, double basePay, double bonusPercent);

    void importFromFile(const std::string& filename);
    void exportToFile(const std::string& filename);

private:
    std::string path;
    struct sqlite3* dbHandle;
    void openDb();
    void closeDb();
};
