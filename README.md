# 🏦 BankingSystem
A lightweight **ImGui-powered** banking simulator in **C++17** with **JSON persistence** — login, accounts, transfers, FX exchange, and savings interest in one desktop app.

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/platform-macOS-black.svg)](#)
[![UI](https://img.shields.io/badge/UI-Dear%20ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![Storage](https://img.shields.io/badge/storage-JSON-green.svg)](https://github.com/nlohmann/json)

**Quick links:**  
· [Key Features](#key-features) · [Getting Started](#getting-started) · [How It Works](#how-it-works) · [Database Format](#database-format) · [Project Structure](#project-structure) · [Notes](#notes)

BankingSystem is an educational desktop banking app built for macOS (Xcode).  
It stores customers & accounts in a JSON database, supports multiple account types (Checking/Savings/FX), logs transfers, and fetches live FX rates via Frankfurter API.

---

## Key Features

### ✅ Authentication & Session
- **Create account** (customer profile + initial accounts)
- **Login** (ID + secret, plus phone verification in flow)
- **Forgot secret** / reset via email verification
- **Session state** managed by a single `AppSession` object

### 💳 Accounts (multi-type)
- **Checking**
- **Savings** (with **interest accrual** logic applied when needed)
- **FX accounts** (per currency), created on demand

### 🔁 Transfers
- Send money **by destination account ID**
- Or send **by recipient name** (first + last)
- **Transfer history** with filters (Today / 7 days / All)
- Persisted transfer logs in JSON (`transfers` list)

### 💱 FX Exchange (Live rates)
- EUR-base exchange using **Frankfurter API**
- Supports **Buy/Sell** directions (EUR→FX, FX→EUR)
- Rates are fetched via system **`curl`** and cached in session

### 🔒 Privacy & Security
- Global **Hide balances** toggle (privacy mode)
- Change secret inside Settings (old → new)

### 🧩 Clean Architecture
- Core logic in `src/core`
- UI screens in `src/ui`
- Persistence layer in `DatabaseManager` with **backward-compatible DB normalization**

---

## Getting Started

### Requirements
- macOS + **Xcode**
- C++17
- OpenGL (macOS framework)
- GLFW
- Dear ImGui (included in `third_party/imgui`)
- `curl` available in system (used for FX rates fetching)

### Run (Xcode)
1. Open `BankingSystem.xcodeproj`
2. Select the target
3. Build & Run (⌘R)

---

## How It Works

### App Flow
- **Main Menu** → Login / Create / Forgot
- After login: **Dashboard** with bottom navigation:
  - Home
  - Exchange
  - Transfers
  - Deals (demo)
  - Settings

### Core Components
- `AppSession`
  - Holds current page, current user, selected tab
  - Stores UI state (inputs, selected account, toggles)
  - Contains FX rates cache and helper utilities (validation, dates)
- `DatabaseManager`
  - Loads/saves JSON
  - Manages customers CRUD
  - Verifies secrets/phone, handles reset/change secret
  - Appends transfer logs and supports history filtering
  - Normalizes DB to support old/new formats
- `Customer`, `Account`
  - Customer profile + vector of accounts
  - Account operations: deposit/withdraw + type-specific fields (Savings rate/date, FX currency)

---

## Database Format

Database lives at:
- `BankingSystem/data/database.json`

The app supports a “new” normalized structure:
- `customers` (map by customer ID)
- `transfers` (array of transfer logs)

There are also test fixtures:
- `BankingSystem/data/test_db.json`

> Tip: keep `*.bak` files out of git.

---

## Project Structure

```text
BankingSystem/
├── BankingSystem.xcodeproj/        # Xcode project (macOS build entrypoint)
└── BankingSystem/
    ├── src/
    │   ├── core/                   # Customer, Account, DatabaseManager, AppSession logic
    │   ├── ui/                     # ImGui screens: Login/Create/Forgot/Dashboard/MainMenu
    │   └── main.cpp                # GLFW + ImGui loop & page routing
    ├── include/                    # headers + nlohmann/json single header
    ├── data/                       # database.json, test_db.json
    └── third_party/imgui/          # Dear ImGui sources
