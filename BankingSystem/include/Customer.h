#pragma once
#include <string>
#include <vector>
#include "Account.h"

class Customer {
private:
    std::string firstName;
    std::string lastName;
    int age;
    std::string email;
    std::string id;
    std::string secretWord;
    std::string phone;
    std::vector<Account> accounts;

public:
    Customer();

    // Backward-compatible (старые тесты/код)
    Customer(const std::string& firstName, const std::string& lastName,
             int age, const std::string& email,
             const std::string& id, const std::string& secretWord);

    // Main
    Customer(const std::string& firstName, const std::string& lastName,
             int age, const std::string& email,
             const std::string& id, const std::string& secretWord,
             const std::string& phone);

    // Getters
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getFullName() const;

    int getAge() const;
    std::string getEmail() const;
    std::string getId() const;
    std::string getSecretWord() const;
    std::string getPhone() const;

    // Setters
    void setFirstName(const std::string& fn);
    void setLastName(const std::string& ln);
    void setAge(int a);
    void setEmail(const std::string& e);
    void setId(const std::string& i);
    void setSecretWord(const std::string& s);
    void setPhone(const std::string& p);

    // Accounts
    void addAccount(const Account& acc);
    std::vector<Account>& getAccounts();
    const std::vector<Account>& getAccounts() const;

    void printInfo() const;
};
