# Banking System (C++/ JSON banking system)

The project is an educational console banking system
written in **C++17** with data storage in **JSON format**
(the [nlohmann/json] library(https://github.com/nlohmann/json )).

The system allows you to create clients, open bank accounts,
conduct transactions, charge interest, and restore passwords.

---

## 🛠 Technologies
- C++17
- nlohmann::json (single-header)
- Xcode / g++
- Working with the file system via `<filesystem>`

---

## 📂 Project structure
BankingSystem/
├── data/
│ └── database.json ← main database
├── include/
│ ├── Account.h
│ ├── Customer.h
│ ├── DatabaseManager.h
│ └── nlohmann/
│ └── json.hpp ← library JSON
├── scr/
│ ├── Account.cpp
│ ├── Customer.cpp
│ ├── DatabaseManager.cpp
│ └── main.cpp
└── docs/
├── README.md
├── ARCHITECTURE.md
├── FLOW.md
├── FEATURES.md
├── DB_FORMAT.md
├── TESTING.md
└── FUTURE.md

---

## ⚙️ Launch
### Via the terminal:
``bash
g++ -std=gnu++17 scr/*.cpp -Iinclude -o bank_app
./bank_app

 Via Xcode:

 Create a Command Line Tool target.

 Add everything .cpp to the Compile Sources section.

 Specify the path $(PROJECT_DIR)/BankingSystem/include in Header Search Paths.

 , Database

 The database is stored in data/database.json.
 Each time you save, the following are created:

 database.json.tmp is a temporary file for atomic writing;

 database.json.bak is a backup copy of the previous version.

 🔐 Security

 You cannot overwrite a non-empty database with empty data;

 All operations with JSON are performed with error checking;

 secretWord is stored in clear text, and will be replaced with hashing in the future.

 ✨ Capabilities

 Client registration;

 Choosing the number of accounts:

 1 → Checking;

 2 → Checking + Savings;

 Login with password verification;

 Accrual of interest on Savings upon entry;

 Deposit, withdrawal, transfer, temporary deposits;

 Password recovery via email;

 Generating unique account IDs;

 Working with JSON without data loss.

 , Author and version

 Author: DanilaS

 Version: 1.0

 Year: 2025
