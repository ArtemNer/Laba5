#include "Payroll.h"

#include <fstream>
#include <sstream>
#include <algorithm>

// ===== Исключения =====

PayrollException::PayrollException(const std::string& msg)
    : std::runtime_error(msg) {}

InvalidRateException::InvalidRateException(const std::string& msg)
    : PayrollException("Invalid rate: " + msg) {}

DuplicateWorkTypeException::DuplicateWorkTypeException(const std::string& msg)
    : PayrollException("Duplicate work type: " + msg) {}

EmptyWorkListException::EmptyWorkListException(const std::string& msg)
    : PayrollException("Work list is empty: " + msg) {}

// ===== Стратегии =====

double NoBonusStrategy::computePay(double basePay) const {
    return basePay;
}

PercentageBonusStrategy::PercentageBonusStrategy(double percent)
    : bonusPercent(percent)
{
    if (bonusPercent < 0.0) {
        throw InvalidRateException("bonus percent must be >= 0");
    }
}

double PercentageBonusStrategy::getPercent() const {
    return bonusPercent;
}

double PercentageBonusStrategy::computePay(double basePay) const {
    return basePay * (1.0 + bonusPercent / 100.0);
}

// ===== WorkTypeBase =====

WorkTypeBase::WorkTypeBase(const std::string& name,
    double basePay,
    double bonusPercent,
    std::shared_ptr<IBonusStrategy> strategy)
    : name(name),
    basePay(basePay),
    bonusPercent(bonusPercent),
    bonusStrategy(strategy)
{
    if (name.empty()) throw InvalidRateException("name must not be empty");
    if (basePay <= 0) throw InvalidRateException("base pay must be > 0");
    if (bonusPercent < 0) throw InvalidRateException("bonus >= 0");
    if (!bonusStrategy) throw InvalidRateException("strategy must not be null");
}

std::string WorkTypeBase::getName() const { return name; }
double WorkTypeBase::getBasePay() const { return basePay; }
double WorkTypeBase::getBonusPercent() const { return bonusPercent; }
double WorkTypeBase::getFinalPay() const { return bonusStrategy->computePay(basePay); }

// ===== PayrollDepartment =====

PayrollDepartment::PayrollDepartment() = default;

bool PayrollDepartment::existsWorkType(const std::string& name) const {
    for (const auto& w : workTypes)
        if (w->getName() == name) return true;
    return false;
}

void PayrollDepartment::addWorkType(const std::string& name,
    double basePay,
    double bonusPercent)
{
    if (existsWorkType(name)) {
        throw DuplicateWorkTypeException(
            "work type '" + name + "' already exists");
    }

    std::shared_ptr<IBonusStrategy> strategy;
    if (bonusPercent == 0.0)
        strategy = std::make_shared<NoBonusStrategy>();
    else
        strategy = std::make_shared<PercentageBonusStrategy>(bonusPercent);

    workTypes.push_back(
        std::make_shared<WorkTypeBase>(name, basePay, bonusPercent, strategy));
}

void PayrollDepartment::updateWorkType(std::size_t index,
    const std::string& name,
    double basePay,
    double bonusPercent)
{
    if (index >= workTypes.size())
        throw PayrollException("index out of range");

    for (std::size_t i = 0; i < workTypes.size(); ++i) {
        if (i != index && workTypes[i]->getName() == name)
            throw DuplicateWorkTypeException("work type '" + name + "' already exists");
    }

    std::shared_ptr<IBonusStrategy> strategy;
    if (bonusPercent == 0.0)
        strategy = std::make_shared<NoBonusStrategy>();
    else
        strategy = std::make_shared<PercentageBonusStrategy>(bonusPercent);

    workTypes[index] =
        std::make_shared<WorkTypeBase>(name, basePay, bonusPercent, strategy);
}

void PayrollDepartment::removeWorkType(std::size_t index)
{
    if (index >= workTypes.size())
        throw PayrollException("index out of range");
    workTypes.erase(workTypes.begin() + index);
}

void PayrollDepartment::clear() {
    workTypes.clear();
}

const std::vector<std::shared_ptr<IWorkType>>&
PayrollDepartment::getWorkTypes() const
{
    return workTypes;
}

double PayrollDepartment::calculateAveragePay() const
{
    if (workTypes.empty())
        throw EmptyWorkListException("cannot calculate average");

    double sum = 0.0;
    for (const auto& w : workTypes) sum += w->getFinalPay();
    return sum / static_cast<double>(workTypes.size());
}

// ===== Файлы =====

static std::string trim(const std::string& s)
{
    std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return std::string();
    std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

void PayrollDepartment::saveToFile(const std::string& filename) const
{
    std::ofstream out(filename.c_str());
    if (!out) throw PayrollException("cannot open file: " + filename);

    for (const auto& w : workTypes) {
        out << w->getName() << ';'
            << w->getBasePay() << ';'
            << w->getBonusPercent() << '\n';
    }
}

void PayrollDepartment::loadFromFile(const std::string& filename)
{
    std::ifstream in(filename.c_str());
    if (!in) throw PayrollException("cannot open file: " + filename);

    workTypes.clear();

    std::string line;
    std::size_t lineNo = 0;

    while (std::getline(in, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string name, baseStr, bonusStr;
        if (!std::getline(iss, name, ';') ||
            !std::getline(iss, baseStr, ';') ||
            !std::getline(iss, bonusStr))
        {
            throw PayrollException("invalid format at line " +
                std::to_string(lineNo));
        }

        name = trim(name);
        baseStr = trim(baseStr);
        bonusStr = trim(bonusStr);

        double basePay = std::stod(baseStr);
        double bonusPercent = std::stod(bonusStr);

        addWorkType(name, basePay, bonusPercent);
    }
}

// ===== Сортировки =====

void PayrollDepartment::sortByName(bool ascending)
{
    std::sort(workTypes.begin(), workTypes.end(),
        [ascending](const std::shared_ptr<IWorkType>& a,
            const std::shared_ptr<IWorkType>& b)
        {
            return ascending ? a->getName() < b->getName()
                : a->getName() > b->getName();
        });
}

void PayrollDepartment::sortByFinalPay(bool ascending)
{
    std::sort(workTypes.begin(), workTypes.end(),
        [ascending](const std::shared_ptr<IWorkType>& a,
            const std::shared_ptr<IWorkType>& b)
        {
            return ascending ? a->getFinalPay() < b->getFinalPay()
                : a->getFinalPay() > b->getFinalPay();
        });
}
