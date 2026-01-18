#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// ===== Исключения =====

class PayrollException : public std::runtime_error {
public:
    explicit PayrollException(const std::string& msg);
};

class InvalidRateException : public PayrollException {
public:
    explicit InvalidRateException(const std::string& msg);
};

class DuplicateWorkTypeException : public PayrollException {
public:
    explicit DuplicateWorkTypeException(const std::string& msg);
};

class EmptyWorkListException : public PayrollException {
public:
    explicit EmptyWorkListException(const std::string& msg);
};

// ===== Стратегии бонусов =====

class IBonusStrategy {
public:
    virtual ~IBonusStrategy() {}
    virtual double computePay(double basePay) const = 0;
};

class NoBonusStrategy : public IBonusStrategy {
public:
    double computePay(double basePay) const override;
};

class PercentageBonusStrategy : public IBonusStrategy {
private:
    double bonusPercent;
public:
    explicit PercentageBonusStrategy(double percent);
    double getPercent() const;
    double computePay(double basePay) const override;
};

// ===== Тип работы =====

class IWorkType {
public:
    virtual ~IWorkType() {}
    virtual std::string getName() const = 0;
    virtual double getBasePay() const = 0;
    virtual double getBonusPercent() const = 0;
    virtual double getFinalPay() const = 0;
};

class WorkTypeBase : public IWorkType {
private:
    std::string name;
    double basePay;
    double bonusPercent;
    std::shared_ptr<IBonusStrategy> bonusStrategy;
public:
    WorkTypeBase(const std::string& name,
        double basePay,
        double bonusPercent,
        std::shared_ptr<IBonusStrategy> strategy);

    std::string getName() const override;
    double getBasePay() const override;
    double getBonusPercent() const override;
    double getFinalPay() const override;
};

// ===== Отдел расчёта зарплаты =====

class PayrollDepartment {
private:
    std::vector<std::shared_ptr<IWorkType>> workTypes;

    bool existsWorkType(const std::string& name) const;

public:
    PayrollDepartment();

    void addWorkType(const std::string& name,
        double basePay,
        double bonusPercent = 0.0);

    void updateWorkType(std::size_t index,
        const std::string& name,
        double basePay,
        double bonusPercent);

    void removeWorkType(std::size_t index);

    void clear();

    const std::vector<std::shared_ptr<IWorkType>>& getWorkTypes() const;

    double calculateAveragePay() const;

    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);

    void sortByName(bool ascending);
    void sortByFinalPay(bool ascending);
};
