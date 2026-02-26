#pragma once
#include <string>
#include <vector>

#include "Customer.h"
#include "Account.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class DatabaseManager {
private:
    std::string filename;


    // Compatibility layer:
    // old style DB: { "123": {...}, "456": {...} }
    // new style DB: { "customers": {...}, "transfers": [...] }
    static json& customersRef(json& root);
    static const json& customersRefConst(const json& root);

public:
    explicit DatabaseManager(const std::string& filename = "data/database.json");

    // Storage
    bool loadAll(json& outJson);
    bool saveAll(const json& j);

    // Customers
    bool customerExists(const std::string& id);
    bool addOrUpdateCustomer(const Customer& customer);
    bool loadCustomer(const std::string& id, Customer& outCustomer);
    bool removeCustomer(const std::string& id);

    bool verifySecret(const std::string& id, const std::string& secret) const;
    bool verifyPhone(const std::string& id, const std::string& phone);
    bool changeSecret(const std::string& id, const std::string& oldSecret, const std::string& newSecret);
    bool resetSecretWithEmail(const std::string& id, const std::string& email, const std::string& newSecret);

    // IMPORTANT: now uses firstName + lastName
    bool findCustomerByName(const std::string& firstName,
                            const std::string& lastName,
                            std::string& outId);

    // Transfers log (global)
    bool appendTransferLog(const json& entry);
    std::vector<json> getTransfersForCustomer(const std::string& customerId, int daysBack /*0=all*/);

    // Helpers
    int generateUniqueAccountId();
    std::vector<int> existingAccountIds();
};
