#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "Customer.h"
#include "Account.h"
#include "DatabaseManager.h"

enum class Page { MainMenu, Login, Create, Forgot, Dashboard };

// Bottom tabs (inside Dashboard)
enum class AppTab { Home, Exchange, Transfers, Deals, Settings };

struct AppSession {
    DatabaseManager db{ "data/database.json" };
    Page page = Page::MainMenu;

    // Logged in
    Customer current;

    // Global UI
    AppTab tab = AppTab::Home;
    bool hideBalances = true; // GLOBAL hide/show balances

    // Toast
    std::string toast;
    double toast_t = 0.0;
    void ShowToast(const std::string& msg);

    // --- Login ---
    std::string loginId;
    std::string loginSecret;
    std::string loginPhone;
    std::string loginError;

    // --- Forgot ---
    std::string fId, fEmail, fNewSecret;
    std::string fMsg;

    // --- Create ---
    std::string cFirstName, cLastName;
    std::string cEmail, cId, cSecret, cPhone;
    int cAge = 18;
    int cOpenCount = 1;

    // --- Home quick actions ---
    double qaAmount = 0.0;
    int qaDenomIdx = 3;
    int qaBillCount = 1;
    int selectedAccIdx = -1;

    // --- Transfers ---
    int trSubPage = 0; // 0=Make, 1=History, 2=About
    int trMode = 0;    // 0=By Account ID, 1=By Name
    int trFromIdx = -1;
    int trDestAccId = 0;
    std::string trFirstName, trLastName;
    double trAmount = 0.0;
    int trHistoryFilter = 1; // 0=Today, 1=7 days, 2=All

    // --- Exchange ---
    double exAmount = 0.0;
    int exTargetIdx = 0;  // index in list
    int exDirection = 0;  // 0=Buy (EUR->FX), 1=Sell (FX->EUR)

    std::unordered_map<std::string, double> exRates; // currency -> rate (EUR base)
    std::string exLastError;
    double exLastFetchT = -1.0;
    double exNextPollT  = 0.0;

    // --- Helpers ---
    static bool validateID(const std::string& id);
    static bool validateEmail(const std::string& email);
    static bool validatePhone(const std::string& phone);

    static std::string todayDate();
    static int daysBetween(const std::string& fromDate, const std::string& toDate);
    static int findAccountIndexById(const std::vector<Account>& accounts, int accId);

    void applySavingsInterestIfNeeded(Customer& cust);

    // FX helpers
    int findCheckingIndex() const;
    int findFXIndexByCurrency(const std::string& cur) const;
    int ensureFXAccount(const std::string& cur); // create if missing, returns idx or -1 on fail
};
