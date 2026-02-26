#include "Customer.h"
#include <iostream>

Customer::Customer()
    : firstName(""), lastName(""),
      age(0), email(""), id(""),
      secretWord(""), phone(""), accounts() {}

Customer::Customer(const std::string& fn, const std::string& ln,
                   int age, const std::string& email,
                   const std::string& id, const std::string& secretWord)
    : firstName(fn), lastName(ln),
      age(age), email(email),
      id(id), secretWord(secretWord),
      phone(""), accounts() {}

Customer::Customer(const std::string& fn, const std::string& ln,
                   int age, const std::string& email,
                   const std::string& id, const std::string& secretWord,
                   const std::string& phone)
    : firstName(fn), lastName(ln),
      age(age), email(email),
      id(id), secretWord(secretWord),
      phone(phone), accounts() {}

std::string Customer::getFirstName() const { return firstName; }
std::string Customer::getLastName() const  { return lastName; }

std::string Customer::getFullName() const {
    if (firstName.empty() && lastName.empty()) return "";
    if (lastName.empty()) return firstName;
    if (firstName.empty()) return lastName;
    return firstName + " " + lastName;
}

int Customer::getAge() const { return age; }
std::string Customer::getEmail() const { return email; }
std::string Customer::getId() const { return id; }
std::string Customer::getSecretWord() const { return secretWord; }
std::string Customer::getPhone() const { return phone; }

void Customer::setFirstName(const std::string& fn) { firstName = fn; }
void Customer::setLastName(const std::string& ln)  { lastName = ln; }
void Customer::setAge(int a) { age = a; }
void Customer::setEmail(const std::string& e) { email = e; }
void Customer::setId(const std::string& i) { id = i; }
void Customer::setSecretWord(const std::string& s) { secretWord = s; }
void Customer::setPhone(const std::string& p) { phone = p; }

void Customer::addAccount(const Account& acc) { accounts.push_back(acc); }
std::vector<Account>& Customer::getAccounts() { return accounts; }
const std::vector<Account>& Customer::getAccounts() const { return accounts; }

void Customer::printInfo() const {
    std::cout << "Customer: " << getFullName()
              << ", Age: " << age
              << ", Email: " << email
              << ", ID: " << id
              << ", Phone: " << phone
              << std::endl;
}
