#include "Account.h"
#include <iostream>
#include <iomanip>

using namespace std;

Account::Account()
    : id(0), type("Unknown"), balance(0.0),
      savingsRate(0.0), lastSavedDate(""),
      currency("") {}

Account::Account(int id, const std::string& type, double balance)
    : id(id), type(type), balance(balance),
      savingsRate(0.0), lastSavedDate(""),
      currency("") {}

int Account::getId() const { return id; }
std::string Account::getType() const { return type; }
double Account::getBalance() const { return balance; }

void Account::setBalance(double b) { balance = b; }
void Account::setType(const std::string& t) { type = t; }

double Account::getSavingsRate() const { return savingsRate; }
void Account::setSavingsRate(double r) { savingsRate = r; }
std::string Account::getLastSavedDate() const { return lastSavedDate; }
void Account::setLastSavedDate(const std::string& d) { lastSavedDate = d; }

std::string Account::getCurrency() const { return currency; }
void Account::setCurrency(const std::string& c) { currency = c; }

void Account::deposit(double amount) {
    if (amount <= 0) {
        cout << "Deposit amount must be positive.\n";
        return;
    }
    balance += amount;
    cout << "Deposited " << fixed << setprecision(2) << amount
         << " to account " << id << ". New balance: " << balance << endl;
}

bool Account::withdraw(double amount) {
    if (amount <= 0) {
        cout << "Withdrawal amount must be positive.\n";
        return false;
    }
    if (amount > balance) {
        cout << "Insufficient funds on account " << id << ".\n";
        return false;
    }
    balance -= amount;
    cout << "Withdrawn " << fixed << setprecision(2) << amount
         << " from account " << id << ". New balance: " << balance << endl;
    return true;
}

void Account::printInfo() const {
    cout << "Account ID: " << id
         << " | Type: " << type;

    if (type == "FX" && !currency.empty()) {
        cout << " (" << currency << ")";
    }

    cout << " | Balance: " << fixed << setprecision(2) << balance;

    if (type == "Savings") {
        cout << " | Rate: " << (savingsRate * 100) << "%";
        if (!lastSavedDate.empty()) cout << " | LastSaved: " << lastSavedDate;
    }
    cout << endl;
}
