#include "AppSession.h"
#include "imgui.h"
#include "imgui_stdlib.h"

#include <vector>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <cmath>

// --------- Rates config (top-10) ---------
static const char* FX_LIST[] = {"USD","GBP","JPY","CHF","CAD","AUD","NZD","SEK","NOK","CNY"};
static const int FX_N = (int)(sizeof(FX_LIST)/sizeof(FX_LIST[0]));

static std::string joinSymbolsCSV() {
    std::string s;
    for (int i = 0; i < FX_N; ++i) {
        if (i) s += ",";
        s += FX_LIST[i];
    }
    return s;
}

// Minimal fetch via curl (Frankfurter)
static bool fetchRatesEUR(AppSession& S) {
    std::string symbols = joinSymbolsCSV();
    std::string url = "https://api.frankfurter.app/latest?from=EUR&to=" + symbols;

    std::string cmd = "curl -s \"" + url + "\"";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) { S.exLastError = "Failed to run curl."; return false; }

    std::string data;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) data += buffer;
    int rc = pclose(pipe);
    (void)rc;

    try {
        auto j = json::parse(data);
        if (!j.contains("rates") || !j["rates"].is_object()) {
            S.exLastError = "Rates payload invalid.";
            return false;
        }
        S.exRates.clear();
        for (auto it = j["rates"].begin(); it != j["rates"].end(); ++it) {
            S.exRates[it.key()] = it.value().get<double>();
        }
        S.exLastError.clear();
        return true;
    } catch (...) {
        S.exLastError = "Failed to parse rates JSON.";
        return false;
    }
}

static std::string moneyStr(double x, bool hide) {
    if (hide) return "HIDDEN"; // ASCII-only (no ????)
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2) << x;
    return ss.str();
}

// Pick destination account for name-transfer: prefer Checking else first
static int pickDestAccountIndex(const Customer& c) {
    const auto& accs = c.getAccounts();
    for (int i = 0; i < (int)accs.size(); ++i)
        if (accs[i].getType() == "Checking") return i;
    return accs.empty() ? -1 : 0;
}

static void drawToast(AppSession& S) {
    if (!S.toast.empty() && ImGui::GetTime() - S.toast_t < 4.0) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.2f,0.9f,0.2f,1), "%s", S.toast.c_str());
    }
}

// ---------------- Home ----------------
static void DrawHome(AppSession& S) {
    ImGui::SeparatorText("Home");

    // Greeting (FIRST+LAST)
    ImGui::Text("Welcome, %s", S.current.getFullName().c_str());

    // Hide/Show balances (global)
    if (ImGui::Button(S.hideBalances ? "Show balances" : "Hide balances")) {
        S.hideBalances = !S.hideBalances;
    }

    ImGui::SeparatorText("Accounts");
    auto& accs = S.current.getAccounts();
    if (S.selectedAccIdx < 0 && !accs.empty()) S.selectedAccIdx = 0;

    for (int i = 0; i < (int)accs.size(); ++i) {
        auto& a = accs[i];

        std::string title = "#" + std::to_string(a.getId()) + "  " + a.getType();
        if (a.getType() == "FX" && !a.getCurrency().empty()) title += " (" + a.getCurrency() + ")";

        if (ImGui::Selectable(title.c_str(), S.selectedAccIdx == i)) S.selectedAccIdx = i;

        ImGui::SameLine(420);
        std::string balLabel;
        if (a.getType() == "FX" && !a.getCurrency().empty()) {
            balLabel = S.hideBalances
                ? "balance hidden"
                : (moneyStr(a.getBalance(), false) + " " + a.getCurrency());
        } else {
            balLabel = S.hideBalances
                ? "balance hidden"
                : (moneyStr(a.getBalance(), false) + " EUR");
        }

        ImGui::Text("%s", balLabel.c_str());
    }

    ImGui::SeparatorText("Quick actions");
    if (ImGui::Button("Go to Transfers")) {
        S.tab = AppTab::Transfers;
        S.trSubPage = 0;
    }

    // Simple deposit/withdraw here
    if (S.selectedAccIdx >= 0 && S.selectedAccIdx < (int)accs.size()) {
        auto& a = accs[S.selectedAccIdx];

        ImGui::SeparatorText("Deposit / Withdraw");
        ImGui::InputDouble("Amount", &S.qaAmount, 0, 0, "%.2f");

        if (ImGui::Button("Deposit##qa")) {
            if (S.qaAmount <= 0) S.ShowToast("Invalid amount.");
            else {
                a.setBalance(a.getBalance() + S.qaAmount);
                S.db.addOrUpdateCustomer(S.current);
                S.ShowToast("Deposit successful.");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Withdraw##qa")) {
            if (S.qaAmount <= 0) S.ShowToast("Invalid amount.");
            else if (S.qaAmount > a.getBalance()) S.ShowToast("Insufficient funds.");
            else {
                a.setBalance(a.getBalance() - S.qaAmount);
                S.db.addOrUpdateCustomer(S.current);
                S.ShowToast("Withdrawal successful.");
            }
        }
    }
}

// ---------------- Exchange ----------------
static void DrawExchange(AppSession& S) {
    ImGui::SeparatorText("Exchange");

    // Poll rates: on start + every 5 seconds
    double nowT = ImGui::GetTime();
    if (S.exLastFetchT < 0.0) {
        fetchRatesEUR(S);
        S.exLastFetchT = nowT;
        S.exNextPollT = nowT + 5.0;
    } else if (nowT >= S.exNextPollT) {
        fetchRatesEUR(S);
        S.exLastFetchT = nowT;
        S.exNextPollT = nowT + 5.0;
    }

    ImGui::Text("Base currency: EUR");
    if (!S.exLastError.empty()) {
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "Rates error: %s", S.exLastError.c_str());
    } else {
        ImGui::Text("Rates updated every 5 seconds (reference rates).");
    }

    ImGui::SeparatorText("Top-10 rates (EUR -> X)");
    for (int i = 0; i < FX_N; ++i) {
        const char* cur = FX_LIST[i];
        double r = S.exRates.count(cur) ? S.exRates[cur] : 0.0;
        if (r > 0.0) ImGui::BulletText("EUR/%s = %.6f", cur, r);
        else ImGui::BulletText("EUR/%s = N/A", cur);
    }

    ImGui::SeparatorText("Currency accounts");
    auto& accs = S.current.getAccounts();
    bool hasAnyFX = false;
    for (auto& a : accs) {
        if (a.getType() == "FX") {
            hasAnyFX = true;

            std::string label = "#" + std::to_string(a.getId()) +
                                " FX (" + a.getCurrency() + ")";
            if (S.hideBalances) label += " — balance hidden";
            else label += " — " + moneyStr(a.getBalance(), false) + " " + a.getCurrency();

            ImGui::BulletText("%s", label.c_str());
        }
    }
    if (!hasAnyFX) ImGui::TextDisabled("No currency accounts yet. Open one below.");

    ImGui::SeparatorText("Open a currency account (only here)");
    static int openIdx = 0;
    ImGui::Combo("Currency", &openIdx, [](void*, int idx, const char** out_text){
        if (idx < 0 || idx >= FX_N) return false;
        *out_text = FX_LIST[idx];
        return true;
    }, nullptr, FX_N);

    if (ImGui::Button("Open FX account")) {
        std::string cur = FX_LIST[openIdx];
        if (S.findFXIndexByCurrency(cur) >= 0) {
            S.ShowToast("FX account already exists for " + cur);
        } else {
            int idx = S.ensureFXAccount(cur);
            if (idx >= 0) S.ShowToast("FX account opened: " + cur);
            else S.ShowToast("Failed to open FX account.");
        }
    }

    ImGui::SeparatorText("Convert (two-way)");
    ImGui::RadioButton("Buy (EUR -> FX)", &S.exDirection, 0); ImGui::SameLine();
    ImGui::RadioButton("Sell (FX -> EUR)", &S.exDirection, 1);

    ImGui::Combo("Target currency", &S.exTargetIdx, [](void*, int idx, const char** out_text){
        if (idx < 0 || idx >= FX_N) return false;
        *out_text = FX_LIST[idx];
        return true;
    }, nullptr, FX_N);

    ImGui::InputDouble("Amount", &S.exAmount, 0, 0, "%.2f");

    std::string cur = FX_LIST[S.exTargetIdx];
    double rate = S.exRates.count(cur) ? S.exRates[cur] : 0.0;

    if (rate <= 0) {
        ImGui::TextDisabled("Rate unavailable for %s (wait next update).", cur.c_str());
        return;
    }

    if (S.exDirection == 0) {
        double fx = S.exAmount * rate;
        ImGui::Text("You pay: %.2f EUR  ->  You receive: %.2f %s", S.exAmount, fx, cur.c_str());
    } else {
        double eur = (rate > 0) ? (S.exAmount / rate) : 0.0;
        ImGui::Text("You pay: %.2f %s  ->  You receive: %.2f EUR", S.exAmount, cur.c_str(), eur);
    }

    if (ImGui::Button("Exchange")) {
        int chkIdx = S.findCheckingIndex();
        if (chkIdx < 0) { S.ShowToast("No Checking account found."); return; }

        int fxIdx = S.findFXIndexByCurrency(cur);
        if (fxIdx < 0) { S.ShowToast("Open FX account for " + cur + " first."); return; }

        Account& checking = S.current.getAccounts()[chkIdx];
        Account& fxAcc    = S.current.getAccounts()[fxIdx];

        if (S.exAmount <= 0) { S.ShowToast("Invalid amount."); return; }

        if (S.exDirection == 0) {
            if (checking.getBalance() < S.exAmount) { S.ShowToast("Insufficient EUR in Checking."); return; }
            double fx = S.exAmount * rate;
            checking.setBalance(checking.getBalance() - S.exAmount);
            fxAcc.setBalance(fxAcc.getBalance() + fx);
            S.db.addOrUpdateCustomer(S.current);
            S.ShowToast("Exchange complete (EUR -> " + cur + ").");
        } else {
            if (fxAcc.getBalance() < S.exAmount) { S.ShowToast("Insufficient FX balance."); return; }
            double eur = S.exAmount / rate;
            fxAcc.setBalance(fxAcc.getBalance() - S.exAmount);
            checking.setBalance(checking.getBalance() + eur);
            S.db.addOrUpdateCustomer(S.current);
            S.ShowToast("Exchange complete (" + cur + " -> EUR).");
        }
    }
}

// ---------------- Transfers ----------------
static void DrawTransfers(AppSession& S) {
    ImGui::SeparatorText("Transfers");

    if (ImGui::Button("Make a Transfer")) S.trSubPage = 0;
    ImGui::SameLine();
    if (ImGui::Button("Transfer History")) S.trSubPage = 1;
    ImGui::SameLine();
    if (ImGui::Button("About Transfers")) S.trSubPage = 2;

    ImGui::Separator();

    if (S.trSubPage == 2) {
        ImGui::TextWrapped(
            "Transfers are allowed only from Checking accounts.\n"
            "You can transfer by destination Account ID, or by recipient first and last name.\n"
            "Failed transfers are saved to history with an error reason.\n"
        );
        return;
    }

    if (S.trSubPage == 1) {
        ImGui::Text("History filters:");
        ImGui::RadioButton("Today", &S.trHistoryFilter, 0); ImGui::SameLine();
        ImGui::RadioButton("Last 7 days", &S.trHistoryFilter, 1); ImGui::SameLine();
        ImGui::RadioButton("All", &S.trHistoryFilter, 2);

        int daysBack = 0;
        if (S.trHistoryFilter == 0) daysBack = 1;
        else if (S.trHistoryFilter == 1) daysBack = 7;
        else daysBack = 0;

        auto items = S.db.getTransfersForCustomer(S.current.getId(), daysBack);
        if (items.empty()) {
            ImGui::TextDisabled("No transfers yet.");
            return;
        }

        for (const auto& e : items) {
            std::string status = e.value("status", "");
            std::string mode = e.value("mode", "");
            double amount = e.value("amount", 0.0);
            int fromAcc = e.value("fromAccId", 0);
            int toAcc = e.value("toAccId", 0);
            std::string target = e.value("target", "");
            std::string error = e.value("error", "");

            ImVec4 col = (status == "ok") ? ImVec4(0.2f,0.9f,0.2f,1) : ImVec4(1,0.3f,0.3f,1);

            ImGui::TextColored(col, "%s | %.2f EUR | from #%d -> #%d | %s",
                status.c_str(), amount, fromAcc, toAcc, mode.c_str());

            if (!target.empty()) ImGui::Text("Target: %s", target.c_str());
            if (!error.empty()) ImGui::Text("Error: %s", error.c_str());
            ImGui::Separator();
        }
        return;
    }

    // Make transfer
    auto& accs = S.current.getAccounts();

    // source list (only checking)
    std::vector<int> checkingIdx;
    std::vector<std::string> labels;
    for (int i = 0; i < (int)accs.size(); ++i) {
        if (accs[i].getType() == "Checking") {
            checkingIdx.push_back(i);
            labels.push_back("#" + std::to_string(accs[i].getId()) + " Checking");
        }
    }

    if (checkingIdx.empty()) {
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "No Checking account found.");
        return;
    }

    static int srcPick = 0;
    if (srcPick < 0) srcPick = 0;
    if (srcPick >= (int)checkingIdx.size()) srcPick = 0;

    std::vector<const char*> cstrs;
    for (auto& s : labels) cstrs.push_back(s.c_str());
    ImGui::Combo("From", &srcPick, cstrs.data(), (int)cstrs.size());
    int fromIdx = checkingIdx[srcPick];

    ImGui::RadioButton("By Account ID", &S.trMode, 0); ImGui::SameLine();
    ImGui::RadioButton("By Name", &S.trMode, 1);

    if (S.trMode == 0) {
        ImGui::InputInt("Destination Account ID", &S.trDestAccId);
    } else {
        ImGui::InputText("First name", &S.trFirstName);
        ImGui::InputText("Last name", &S.trLastName);
    }
    ImGui::InputDouble("Amount (EUR)", &S.trAmount, 0, 0, "%.2f");

    if (ImGui::Button("Send")) {
        auto logBase = json::object();
        logBase["fromCustomerId"] = S.current.getId();
        logBase["fromAccId"] = S.current.getAccounts()[fromIdx].getId();
        logBase["amount"] = S.trAmount;
        logBase["mode"] = (S.trMode == 0) ? "by_account_id" : "by_name";

        auto fail = [&](const std::string& err, const std::string& target){
            json e = logBase;
            e["status"] = "failed";
            e["error"] = err;
            e["target"] = target;
            e["toCustomerId"] = "";
            e["toAccId"] = 0;
            S.db.appendTransferLog(e);
            S.ShowToast(err);
        };

        if (S.trAmount <= 0) { fail("Invalid amount.", ""); return; }

        Account& fromAcc = S.current.getAccounts()[fromIdx];
        if (fromAcc.getBalance() < S.trAmount) { fail("Insufficient funds.", ""); return; }

        std::string destCustId;
        int destAccId = 0;
        std::string targetLabel;

        if (S.trMode == 0) {
            if (S.trDestAccId <= 0) { fail("Enter destination Account ID.", ""); return; }
            targetLabel = std::to_string(S.trDestAccId);

            // search in DB
            json all; S.db.loadAll(all);
            const json& custs = all.contains("customers") ? all["customers"] : all;

            bool found = false;
            for (auto it = custs.begin(); it != custs.end() && !found; ++it) {
                auto& cj = it.value();
                if (!cj.contains("accounts")) continue;
                for (auto& a : cj["accounts"]) {
                    if (a.value("accId", 0) == S.trDestAccId) {
                        found = true; destCustId = it.key(); destAccId = S.trDestAccId; break;
                    }
                }
            }
            if (!found) { fail("Destination account not found.", targetLabel); return; }

        } else {
            targetLabel = S.trFirstName + " " + S.trLastName;
            if (S.trFirstName.empty() || S.trLastName.empty()) { fail("Enter first and last name.", targetLabel); return; }

            if (!S.db.findCustomerByName(S.trFirstName, S.trLastName, destCustId)) {
                fail("Recipient not found.", targetLabel);
                return;
            }

            Customer destCust;
            if (!S.db.loadCustomer(destCustId, destCust)) { fail("Failed to load recipient.", targetLabel); return; }
            int idxPick = pickDestAccountIndex(destCust);
            if (idxPick < 0) { fail("Recipient has no accounts.", targetLabel); return; }
            destAccId = destCust.getAccounts()[idxPick].getId();
        }

        // perform transfer
        Customer destCust;
        if (!S.db.loadCustomer(destCustId, destCust)) { fail("Failed to load recipient.", targetLabel); return; }

        int destIdx = AppSession::findAccountIndexById(destCust.getAccounts(), destAccId);
        if (destIdx < 0) { fail("Destination account vanished.", targetLabel); return; }

        fromAcc.setBalance(fromAcc.getBalance() - S.trAmount);
        destCust.getAccounts()[destIdx].setBalance(destCust.getAccounts()[destIdx].getBalance() + S.trAmount);

        S.db.addOrUpdateCustomer(S.current);
        S.db.addOrUpdateCustomer(destCust);

        json ok = logBase;
        ok["status"] = "ok";
        ok["error"] = "";
        ok["target"] = targetLabel;
        ok["toCustomerId"] = destCustId;
        ok["toAccId"] = destAccId;
        S.db.appendTransferLog(ok);

        S.ShowToast("Transfer successful.");
    }
}

// ---------------- Deals ----------------
static void DrawDeals(AppSession& S) {
    (void)S;
    ImGui::SeparatorText("Deals");
    ImGui::TextWrapped("Demo offers (no real integration):");

    ImGui::Separator();
    ImGui::BulletText("0%% fee on transfers this week (demo)");
    ImGui::BulletText("5%% cashback on groceries (demo)");
    ImGui::BulletText("Invite a friend: earn 10 EUR (demo)");
    ImGui::Separator();

    ImGui::TextDisabled("In a real app, deals would be personalized.");
}

// ---------------- Settings ----------------
static void DrawSettings(AppSession& S) {
    ImGui::SeparatorText("Settings");

    ImGui::Text("Profile");
    ImGui::BulletText("Name: %s", S.current.getFullName().c_str());
    ImGui::BulletText("Email: %s", S.current.getEmail().c_str());
    ImGui::BulletText("Phone: %s", S.current.getPhone().c_str());
    ImGui::BulletText("ID: %s", S.current.getId().c_str());

    ImGui::SeparatorText("Privacy");
    ImGui::Checkbox("Hide balances (global)", &S.hideBalances);

    ImGui::SeparatorText("Security");
    static std::string oldS, newS;
    ImGui::InputText("Old secret", &oldS, ImGuiInputTextFlags_Password);
    ImGui::InputText("New secret", &newS, ImGuiInputTextFlags_Password);

    if (ImGui::Button("Change secret")) {
        if (oldS.empty() || newS.empty()) S.ShowToast("Fill both fields.");
        else if (S.db.changeSecret(S.current.getId(), oldS, newS)) {
            S.current.setSecretWord(newS);
            S.db.addOrUpdateCustomer(S.current);
            oldS.clear(); newS.clear();
            S.ShowToast("Secret updated.");
        } else {
            S.ShowToast("Failed (old secret mismatch).");
        }
    }

    ImGui::SeparatorText("Session");
    if (ImGui::Button("Logout")) {
        S.db.addOrUpdateCustomer(S.current);
        AppSession fresh;
        S = fresh;
        S.ShowToast("Logged out.");
    }
}

// ---------------- Bottom tab bar ----------------
static void DrawBottomTabs(AppSession& S) {
    ImGui::Separator();
    if (ImGui::Button("Home")) S.tab = AppTab::Home;
    ImGui::SameLine();
    if (ImGui::Button("Exchange")) S.tab = AppTab::Exchange;
    ImGui::SameLine();
    if (ImGui::Button("Transfers")) S.tab = AppTab::Transfers;
    ImGui::SameLine();
    if (ImGui::Button("Deals")) S.tab = AppTab::Deals;
    ImGui::SameLine();
    if (ImGui::Button("Settings")) S.tab = AppTab::Settings;
}

void DrawDashboard(AppSession& S) {
    ImGui::Text("ABC Banking  |  Logged in: %s", S.current.getFullName().c_str());
    ImGui::Separator();

    switch (S.tab) {
        case AppTab::Home:      DrawHome(S); break;
        case AppTab::Exchange:  DrawExchange(S); break;
        case AppTab::Transfers: DrawTransfers(S); break;
        case AppTab::Deals:     DrawDeals(S); break;
        case AppTab::Settings:  DrawSettings(S); break;
    }

    drawToast(S);
    DrawBottomTabs(S);
}
