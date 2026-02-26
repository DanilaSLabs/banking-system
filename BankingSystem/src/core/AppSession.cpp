#include "imgui.h"
#include "AppSession.h"

#include <cctype>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

void AppSession::ShowToast(const std::string& msg) {
    toast = msg;
    toast_t = (double)ImGui::GetTime();
}

static bool isDigitsOnly(const std::string& s) {
    if (s.empty()) return false;
    for (unsigned char c : s) if (!std::isdigit(c)) return false;
    return true;
}

bool AppSession::validateID(const std::string& id) {
    const int MIN_ID_LEN = 8;
    const int MAX_ID_LEN = 10;
    return isDigitsOnly(id) && (int)id.size() >= MIN_ID_LEN && (int)id.size() <= MAX_ID_LEN;
}

bool AppSession::validateEmail(const std::string& email) {
    auto at = email.find('@');
    if (at == std::string::npos || at == 0 || at + 1 >= email.size()) return false;
    auto dot = email.find('.', at);
    if (dot == std::string::npos || dot + 1 >= email.size()) return false;
    return true;
}

bool AppSession::validatePhone(const std::string& phone) {
    int digits = 0;
    for (unsigned char c : phone) {
        if (std::isdigit(c)) { digits++; continue; }
        if (c == '+' || c == ' ' || c == '-' || c == '(' || c == ')') continue;
        return false;
    }
    return digits >= 7;
}

std::string AppSession::todayDate() {
    using namespace std::chrono;
    auto now = system_clock::now();
    time_t t = system_clock::to_time_t(now);
    std::tm buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&buf, &t);
#else
    localtime_r(&t, &buf);
#endif
    std::ostringstream ss; ss << std::put_time(&buf, "%Y-%m-%d");
    return ss.str();
}

int AppSession::daysBetween(const std::string& fromDate, const std::string& toDate) {
    std::tm tm1 = {}, tm2 = {};
    std::istringstream ss1(fromDate), ss2(toDate);
    ss1 >> std::get_time(&tm1, "%Y-%m-%d");
    ss2 >> std::get_time(&tm2, "%Y-%m-%d");
    if (ss1.fail() || ss2.fail()) return 0;
    std::time_t t1 = std::mktime(&tm1);
    std::time_t t2 = std::mktime(&tm2);
    double d = std::difftime(t2, t1) / (60*60*24);
    return (int)d;
}

int AppSession::findAccountIndexById(const std::vector<Account>& accounts, int accId) {
    for (size_t i = 0; i < accounts.size(); ++i)
        if (accounts[i].getId() == accId) return (int)i;
    return -1;
}

void AppSession::applySavingsInterestIfNeeded(Customer& cust) {
    constexpr double DEFAULT_SAVINGS_RATE = 0.15;
    std::string now = todayDate();
    for (auto& acc : cust.getAccounts()) {
        if (acc.getType() == "Savings") {
            std::string last = acc.getLastSavedDate();
            if (last.empty()) { acc.setLastSavedDate(now); continue; }
            int days = daysBetween(last, now);
            if (days <= 0) continue;
            double rate = acc.getSavingsRate();
            if (rate <= 0) rate = DEFAULT_SAVINGS_RATE;
            double interest = acc.getBalance() * rate * (double(days)/365.0);
            if (interest > 0.0) acc.setBalance(acc.getBalance() + interest);
            acc.setLastSavedDate(now);
        }
    }
}

int AppSession::findCheckingIndex() const {
    const auto& accs = current.getAccounts();
    for (int i = 0; i < (int)accs.size(); ++i)
        if (accs[i].getType() == "Checking") return i;
    return -1;
}

int AppSession::findFXIndexByCurrency(const std::string& cur) const {
    const auto& accs = current.getAccounts();
    for (int i = 0; i < (int)accs.size(); ++i)
        if (accs[i].getType() == "FX" && accs[i].getCurrency() == cur) return i;
    return -1;
}

int AppSession::ensureFXAccount(const std::string& cur) {
    int idx = findFXIndexByCurrency(cur);
    if (idx >= 0) return idx;

    int newId = db.generateUniqueAccountId();
    Account fx(newId, "FX", 0.0);
    fx.setCurrency(cur);
    current.addAccount(fx);
    db.addOrUpdateCustomer(current);

    return findFXIndexByCurrency(cur);
}
