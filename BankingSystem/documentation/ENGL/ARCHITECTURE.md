# Architecture of the Banking System

## General scheme

+----------------+
| main.cpp |
| (menu, UI) |
+--------+-------+
|
v
+----------------+
| DatabaseManager|
| (working with JSON)|
+--------+-------+
|
v
+----------------+
| Customer |
| (clients) |
+--------+-------+
|
v
+----------------+
| Account |
| (accounts) |
+----------------+


---

## Class relationships
| Class | Description | Uses |
|-------|-----------|------------|
| **Account** | Bank account model (ID, type, balance, rate) | — |
| **Customer** | The bank's client, contains the list `Account` | Account |
| **DatabaseManager** | JSON database management, ID | Customer, Account generation |
| **main** | Interface and logic of user interaction | Everything |

---

## Libraries and technologies
- `<fstream>` — working with files  
- `<filesystem>` — folder creation, backups  
- `<nlohmann/json.hpp>` — serialization and deserialization of data  
- `<chrono>` — interest accrual dates  
- `<iomanip>` — output formatting  

---

## Principles of architecture
- Modularity (each class performs its own task)
- Minimal dependencies
- Secure file management
- Possibility of expansion (new account types, credits, GUI)
