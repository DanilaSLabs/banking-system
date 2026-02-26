#pragma once
#include <string>

class Account {
private:
    int id;
    std::string type;      // "Checking", "Savings", "FX"
    double balance;

    // Savings
    double savingsRate;
    std::string lastSavedDate;

    // FX
    std::string currency;  // e.g. "USD", "JPY" (only meaningful when type == "FX")

public:
    Account();
    Account(int id, const std::string& type, double balance);

    int getId() const;
    std::string getType() const;
    double getBalance() const;

    void setBalance(double b);
    void setType(const std::string& t);

    // Savings
    double getSavingsRate() const;
    void setSavingsRate(double r);
    std::string getLastSavedDate() const;
    void setLastSavedDate(const std::string& d);

    // FX
    std::string getCurrency() const;
    void setCurrency(const std::string& c);

    // ops
    void deposit(double amount);
    bool withdraw(double amount);

    void printInfo() const;
};
