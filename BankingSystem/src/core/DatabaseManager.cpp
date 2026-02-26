#include "DatabaseManager.h"

#include <fstream>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

// ---------------------- helpers ----------------------
static inline std::string trim_copy(std::string s) {
    auto notSpace = [](unsigned char c){ return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static inline std::string lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

static long long nowEpochMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

static int daysBackFromNow(long long tsMs) {
    using namespace std::chrono;
    long long nowMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    long long diff = nowMs - tsMs;
    if (diff < 0) return 0;
    return (int)(diff / (1000LL * 60 * 60 * 24));
}

// Обязательный формат БД (новый)
static json makeEmptyDb() {
    json root = json::object();
    root["customers"] = json::object();
    root["transfers"] = json::array();
    return root;
}

// Нормализация root: гарантирует customers/transfers
static void normalizeDb(json& root) {
    if (!root.is_object()) root = makeEmptyDb();

    // поддержка старого формата (когда root == customers map)
    if (!root.contains("customers")) {
        json customers = json::object();

        // если root выглядит как map клиентов (ключи = id)
        for (auto it = root.begin(); it != root.end(); ++it) {
            if (it.key() == "transfers") continue;
            customers[it.key()] = it.value();
        }

        json transfers = json::array();
        if (root.contains("transfers") && root["transfers"].is_array())
            transfers = root["transfers"];

        root = json::object();
        root["customers"] = customers;
        root["transfers"] = transfers;
    }

    if (!root["customers"].is_object())
        root["customers"] = json::object();

    if (!root.contains("transfers") || !root["transfers"].is_array())
        root["transfers"] = json::array();
}

static bool fileExists(const std::string& path) {
    std::error_code ec;
    return fs::exists(fs::path(path), ec);
}

static bool fileEmpty(const std::string& path) {
    std::error_code ec;
    auto sz = fs::file_size(fs::path(path), ec);
    if (ec) return true;
    return sz == 0;
}

static void ensureParentDir(const std::string& filename) {
    fs::path p(filename);
    std::error_code ec;
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path(), ec);
    }
}

// ---------------------- DatabaseManager ----------------------
DatabaseManager::DatabaseManager(const std::string& filename)
: filename(filename) {
    // Гарантируем, что папка под БД существует
    ensureParentDir(this->filename);

    // Гарантируем, что сама БД существует и валидна
    json root;
    loadAll(root); // loadAll сам создаст если нет
}

// customersRef / customersRefConst должны возвращать root["customers"]
json& DatabaseManager::customersRef(json& root) {
    normalizeDb(root);
    return root["customers"];
}

const json& DatabaseManager::customersRefConst(const json& root) {
    // const-версия без модификаций: аккуратно
    static json empty = json::object();

    if (!root.is_object()) return empty;
    if (root.contains("customers") && root["customers"].is_object())
        return root["customers"];

    // legacy fallback: treat root as customers map (best effort)
    return root;
}

// ---------------------- load/save ----------------------
bool DatabaseManager::loadAll(json& outJson) {
    ensureParentDir(filename);

    // 1) Файла нет -> создаём новый
    if (!fileExists(filename)) {
        outJson = makeEmptyDb();
        return saveAll(outJson);
    }

    // 2) Файл есть, но пустой -> создаём новый
    if (fileEmpty(filename)) {
        outJson = makeEmptyDb();
        return saveAll(outJson);
    }

    // 3) Пытаемся прочитать JSON
    std::ifstream in(filename);
    if (!in) {
        // странный кейс: файл существует, но не открывается -> создаём
        outJson = makeEmptyDb();
        return saveAll(outJson);
    }

    try {
        in >> outJson;
        normalizeDb(outJson);
        return true;
    } catch (...) {
        // 4) Битый JSON -> переименовать и создать новый
        std::error_code ec;
        fs::path bad = fs::path(filename).concat(".corrupt");
        fs::rename(filename, bad, ec);

        outJson = makeEmptyDb();
        return saveAll(outJson);
    }
}

bool DatabaseManager::saveAll(const json& jIn) {
    ensureParentDir(filename);

    json j = jIn;
    normalizeDb(j);

    // atomic save: tmp -> filename, плюс bak
    const std::string tmp = filename + ".tmp";
    const std::string bak = filename + ".bak";

    // 1) write tmp
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) return false;
        out << std::setw(4) << j << std::endl;
        if (!out.good()) return false;
    }

    // 2) create backup of current
    if (fileExists(filename)) {
        std::ifstream src(filename, std::ios::binary);
        std::ofstream dst(bak, std::ios::binary | std::ios::trunc);
        if (src && dst) dst << src.rdbuf();
    }

    // 3) replace
    std::error_code ec;
    fs::rename(tmp, filename, ec);
    if (ec) {
        // если rename не удалось (например другой диск) -> fallback copy+remove
        std::ifstream src(tmp, std::ios::binary);
        std::ofstream dst(filename, std::ios::binary | std::ios::trunc);
        if (!src || !dst) return false;
        dst << src.rdbuf();
        dst.close();
        std::remove(tmp.c_str());
    }
    return true;
}

// ---------------------- Customers ----------------------
bool DatabaseManager::customerExists(const std::string& id) {
    json root;
    if (!loadAll(root)) return false;
    const auto& custs = customersRefConst(root);
    return custs.contains(id);
}

bool DatabaseManager::addOrUpdateCustomer(const Customer& customer) {
    json root;
    if (!loadAll(root)) return false;
    json& custs = customersRef(root);

    json c = json::object();
    c["firstName"]  = customer.getFirstName();
    c["lastName"]   = customer.getLastName();
    c["name"]       = customer.getFullName(); // для читаемости/совместимости
    c["age"]        = customer.getAge();
    c["email"]      = customer.getEmail();
    c["secretWord"] = customer.getSecretWord();
    c["phone"]      = customer.getPhone();

    c["accounts"] = json::array();
    for (const auto& acc : customer.getAccounts()) {
        json a = json::object();
        a["accId"]   = acc.getId();
        a["type"]    = acc.getType();
        a["balance"] = acc.getBalance();

        if (acc.getType() == "Savings") {
            a["savingsRate"]   = acc.getSavingsRate();
            a["lastSavedDate"] = acc.getLastSavedDate();
        }
        if (acc.getType() == "FX") {
            a["currency"] = acc.getCurrency();
        }

        c["accounts"].push_back(a);
    }

    custs[customer.getId()] = c;
    return saveAll(root);
}

static inline void deriveNamesFromLegacy(const json& cust, std::string& outFirst, std::string& outLast) {
    outFirst = cust.value("firstName", "");
    outLast  = cust.value("lastName", "");
    if (!outFirst.empty() || !outLast.empty()) return;

    std::string full = trim_copy(cust.value("name", ""));
    if (full.empty()) return;

    // простой split: first = first word, last = last word
    std::string first, last;
    size_t sp = full.find(' ');
    if (sp == std::string::npos) {
        first = full;
        last = "";
    } else {
        first = trim_copy(full.substr(0, sp));
        last  = trim_copy(full.substr(full.find_last_of(' ') + 1));
    }
    outFirst = first;
    outLast  = last;
}

bool DatabaseManager::loadCustomer(const std::string& id, Customer& outCustomer) {
    json root;
    if (!loadAll(root)) return false;

    const auto& custs = customersRefConst(root);
    if (!custs.contains(id)) return false;

    const json& cust = custs[id];

    std::string fn, ln;
    deriveNamesFromLegacy(cust, fn, ln);

    int age            = cust.value("age", 0);
    std::string email  = cust.value("email", "");
    std::string secret = cust.value("secretWord", "");
    std::string phone  = cust.value("phone", "");

    outCustomer = Customer(fn, ln, age, email, id, secret, phone);

    if (cust.contains("accounts") && cust["accounts"].is_array()) {
        for (const auto& a : cust["accounts"]) {
            int accId = a.value("accId", 0);
            std::string type = a.value("type", "Checking");
            double bal = a.value("balance", 0.0);

            Account acc(accId, type, bal);

            if (type == "Savings") {
                acc.setSavingsRate(a.value("savingsRate", 0.15));
                acc.setLastSavedDate(a.value("lastSavedDate", ""));
            }
            if (type == "FX") {
                acc.setCurrency(a.value("currency", ""));
            }

            outCustomer.addAccount(acc);
        }
    }
    return true;
}

bool DatabaseManager::removeCustomer(const std::string& id) {
    json root;
    if (!loadAll(root)) return false;
    json& custs = customersRef(root);

    if (!custs.contains(id)) return false;
    custs.erase(id);
    return saveAll(root);
}

bool DatabaseManager::verifySecret(const std::string& id, const std::string& secret) const {
    json root;
    if (!const_cast<DatabaseManager*>(this)->loadAll(root)) return false;
    const auto& custs = customersRefConst(root);
    if (!custs.contains(id)) return false;
    return custs[id].value("secretWord", "") == secret;
}

bool DatabaseManager::verifyPhone(const std::string& id, const std::string& phone) {
    json root;
    if (!loadAll(root)) return false;
    const auto& custs = customersRefConst(root);
    if (!custs.contains(id)) return false;
    return custs[id].value("phone", "") == phone;
}

bool DatabaseManager::changeSecret(const std::string& id,
                                   const std::string& oldSecret,
                                   const std::string& newSecret) {
    json root;
    if (!loadAll(root)) return false;
    json& custs = customersRef(root);

    if (!custs.contains(id)) return false;
    if (custs[id].value("secretWord", "") != oldSecret) return false;

    custs[id]["secretWord"] = newSecret;
    return saveAll(root);
}

bool DatabaseManager::resetSecretWithEmail(const std::string& id,
                                           const std::string& email,
                                           const std::string& newSecret) {
    json root;
    if (!loadAll(root)) return false;
    json& custs = customersRef(root);

    if (!custs.contains(id)) return false;
    if (custs[id].value("email", "") != email) return false;

    custs[id]["secretWord"] = newSecret;
    return saveAll(root);
}

// ---------------------- findCustomerByName ----------------------
bool DatabaseManager::findCustomerByName(const std::string& firstName,
                                        const std::string& lastName,
                                        std::string& outId) {
    json root;
    if (!loadAll(root)) return false;
    const auto& custs = customersRefConst(root);

    std::string fn = lower_copy(trim_copy(firstName));
    std::string ln = lower_copy(trim_copy(lastName));
    if (fn.empty() || ln.empty()) return false;

    for (auto it = custs.begin(); it != custs.end(); ++it) {
        const std::string id = it.key();
        const auto& cust = it.value();

        std::string cfn = lower_copy(trim_copy(cust.value("firstName", "")));
        std::string cln = lower_copy(trim_copy(cust.value("lastName", "")));
        if (!cfn.empty() || !cln.empty()) {
            if (cfn == fn && cln == ln) { outId = id; return true; }
            continue;
        }

        // legacy fallback: compare name
        std::string full = lower_copy(trim_copy(cust.value("name", "")));
        if (full == (fn + " " + ln)) { outId = id; return true; }
    }
    return false;
}

// ---------------------- transfers log ----------------------
bool DatabaseManager::appendTransferLog(const json& entry) {
    json root;
    if (!loadAll(root)) return false;
    normalizeDb(root);

    json e = entry;
    if (!e.contains("ts")) e["ts"] = nowEpochMs();

    root["transfers"].push_back(e);
    return saveAll(root);
}

std::vector<json> DatabaseManager::getTransfersForCustomer(const std::string& customerId,
                                                           int daysBack /*0=all*/) {
    std::vector<json> out;
    json root;
    if (!loadAll(root)) return out;
    normalizeDb(root);

    const auto& arr = root["transfers"];
    if (!arr.is_array()) return out;

    for (const auto& e : arr) {
        std::string fromId = e.value("fromCustomerId", "");
        std::string toId   = e.value("toCustomerId", "");
        if (fromId != customerId && toId != customerId) continue;

        if (daysBack > 0) {
            long long ts = e.value("ts", 0LL);
            if (daysBackFromNow(ts) > daysBack) continue;
        }
        out.push_back(e);
    }

    std::sort(out.begin(), out.end(),
              [](const json& a, const json& b){
                  return a.value("ts", 0LL) > b.value("ts", 0LL);
              });
    return out;
}

// ---------------------- account id helpers ----------------------
int DatabaseManager::generateUniqueAccountId() {
    json root;
    if (!loadAll(root)) {
        return (std::rand() % 900000) + 100000;
    }
    normalizeDb(root);

    const auto& custs = customersRefConst(root);

    std::vector<int> used;
    for (auto it = custs.begin(); it != custs.end(); ++it) {
        const auto& cust = it.value();
        if (!cust.contains("accounts") || !cust["accounts"].is_array()) continue;
        for (const auto& a : cust["accounts"]) {
            used.push_back(a.value("accId", 0));
        }
    }

    auto exists = [&](int x){
        return std::find(used.begin(), used.end(), x) != used.end();
    };

    int candidate = (std::rand() % 900000) + 100000;
    int tries = 0;
    while (exists(candidate) && tries < 200000) {
        candidate = (std::rand() % 900000) + 100000;
        ++tries;
    }
    return candidate;
}

std::vector<int> DatabaseManager::existingAccountIds() {
    std::vector<int> out;
    json root;
    if (!loadAll(root)) return out;
    normalizeDb(root);

    const auto& custs = customersRefConst(root);
    for (auto it = custs.begin(); it != custs.end(); ++it) {
        const auto& cust = it.value();
        if (!cust.contains("accounts") || !cust["accounts"].is_array()) continue;
        for (const auto& a : cust["accounts"]) {
            int id = a.value("accId", 0);
            if (id > 0) out.push_back(id);
        }
    }

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}
