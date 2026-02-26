#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <filesystem>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <ctime>

#include "include/Account.h"
#include "include/Customer.h"
#include "include/DatabaseManager.h"

using namespace std;
namespace fs = std::filesystem;

#define TASSERT(expr) do { if(!(expr)){ cerr<<"[FAIL] "<<__FUNCTION__<<" line "<<__LINE__<<": " #expr "\n"; exit(1);} } while(0)
#define TPASS() do { cout<<"[PASS] "<<__FUNCTION__<<"\n"; } while(0)

static const string TEST_DB = "data/test_db.json";

static void wipeDbArtifacts(const string& base) {
    fs::remove(base);
    fs::remove(base + ".bak");
    fs::remove(base + ".tmp");
}

static string todayISO() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm buf{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&buf,&t);
#else
    localtime_r(&t,&buf);
#endif
    ostringstream ss; ss<<put_time(&buf,"%Y-%m-%d"); return ss.str();
}

static int daysBetween(const string& fromDate, const string& toDate) {
    tm tm1{}, tm2{}; istringstream s1(fromDate), s2(toDate);
    s1>>get_time(&tm1,"%Y-%m-%d"); s2>>get_time(&tm2,"%Y-%m-%d");
    if(s1.fail()||s2.fail()) return 0;
    time_t t1=mktime(&tm1), t2=mktime(&tm2);
    return int(difftime(t2,t1)/(60*60*24));
}

// 1. Создание клиента + загрузка
static void test_CreateAndLoadCustomer() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);

    Customer c("Alice",30,"alice@ex.com","10000001","rose");
    Account ch(db.generateUniqueAccountId(),"Checking",100.0);
    Account sv(db.generateUniqueAccountId(),"Savings",200.0);
    sv.setSavingsRate(0.15); sv.setLastSavedDate(todayISO());
    c.addAccount(ch); c.addAccount(sv);

    TASSERT(db.addOrUpdateCustomer(c));
    TASSERT(fs::exists(TEST_DB));

    Customer loaded;
    TASSERT(db.loadCustomer("10000001",loaded));
    TASSERT(loaded.getName()=="Alice");
    TASSERT((int)loaded.getAccounts().size()==2);
    TPASS();
}

// 2. Обновление клиента
static void test_UpdateOnlyOwnBlock() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);

    Customer a("A",20,"a@e","11111111","x");
    a.addAccount(Account(db.generateUniqueAccountId(),"Checking",10));
    Customer b("B",21,"b@e","22222222","y");
    b.addAccount(Account(db.generateUniqueAccountId(),"Checking",20));
    TASSERT(db.addOrUpdateCustomer(a));
    TASSERT(db.addOrUpdateCustomer(b));

    Customer a2("A2",22,"a2@e","11111111","x2");
    a2.addAccount(Account(db.generateUniqueAccountId(),"Checking",30));
    TASSERT(db.addOrUpdateCustomer(a2));

    Customer outA,outB;
    TASSERT(db.loadCustomer("11111111",outA));
    TASSERT(db.loadCustomer("22222222",outB));
    TASSERT(outA.getName()=="A2");
    TASSERT(outB.getName()=="B");
    TPASS();
}

// 3. Удаление клиента
static void test_RemoveCustomer() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);
    Customer c("C",33,"c@e","33333333","s");
    c.addAccount(Account(db.generateUniqueAccountId(),"Checking",50));
    TASSERT(db.addOrUpdateCustomer(c));
    TASSERT(db.customerExists("33333333"));
    TASSERT(db.removeCustomer("33333333"));
    TASSERT(!db.customerExists("33333333"));
    TPASS();
}

// 4. Правило Checking / Savings
static void test_AccountCreationRule() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);
    Customer one("One",25,"1@e","44444444","p");
    one.addAccount(Account(db.generateUniqueAccountId(),"Checking",0));
    TASSERT(db.addOrUpdateCustomer(one));

    Customer two("Two",26,"2@e","55555555","p");
    two.addAccount(Account(db.generateUniqueAccountId(),"Checking",0));
    Account s(db.generateUniqueAccountId(),"Savings",0);
    s.setSavingsRate(0.15); s.setLastSavedDate(todayISO());
    two.addAccount(s);
    TASSERT(db.addOrUpdateCustomer(two));

    Customer out1,out2;
    TASSERT(db.loadCustomer("44444444",out1));
    TASSERT(db.loadCustomer("55555555",out2));
    TASSERT((int)out1.getAccounts().size()==1);
    TASSERT((int)out2.getAccounts().size()==2);
    TPASS();
}

// 5. Депозит / Снятие
static void test_DepositWithdrawEdges() {
    wipeDbArtifacts(TEST_DB);
    Account a(10,"Checking",100.0);
    a.deposit(-5);                 // игнор
    TASSERT(a.getBalance()==100.0);
    TASSERT(a.withdraw(1000.0)==false);
    TASSERT(a.getBalance()==100.0);
    TASSERT(a.withdraw(40.0)==true);
    TASSERT(a.getBalance()==60.0);
    a.deposit(0.0);
    TASSERT(a.getBalance()==60.0);
    TPASS();
}

// 6. Перевод между клиентами
static void test_TransferLikeFlow() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);

    Customer from("From",29,"f@e","66666666","s");
    Account fromCh(db.generateUniqueAccountId(),"Checking",150.0);
    from.addAccount(fromCh);
    TASSERT(db.addOrUpdateCustomer(from));

    Customer to("To",30,"t@e","77777777","s");
    int destId = db.generateUniqueAccountId();
    Account toAcc(destId,"Savings",5.0);
    to.addAccount(toAcc);
    TASSERT(db.addOrUpdateCustomer(to));

    Customer reFrom; db.loadCustomer("66666666",reFrom);
    reFrom.getAccounts()[0].withdraw(40.0);
    Customer reTo; db.loadCustomer("77777777",reTo);
    reTo.getAccounts()[0].deposit(40.0);
    TASSERT(db.addOrUpdateCustomer(reFrom));
    TASSERT(db.addOrUpdateCustomer(reTo));

    Customer cf, ct;
    db.loadCustomer("66666666",cf);
    db.loadCustomer("77777777",ct);
    TASSERT(cf.getAccounts()[0].getBalance()==110.0);
    TASSERT(ct.getAccounts()[0].getBalance()==45.0);
    TPASS();
}

// 7. Проценты
static void test_SavingsInterest() {
    Account s(1,"Savings",1000.0);
    s.setSavingsRate(0.365);
    auto now = chrono::system_clock::now();
    auto y = now - chrono::hours(24);
    time_t ty = chrono::system_clock::to_time_t(y);
    tm buf{}; localtime_r(&ty,&buf);
    ostringstream ss; ss<<put_time(&buf,"%Y-%m-%d");
    s.setLastSavedDate(ss.str());

    string today = todayISO();
    int days = daysBetween(s.getLastSavedDate(), today);
    double before = s.getBalance();
    if (days>0) {
        double interest = before * s.getSavingsRate() * (double(days)/365.0);
        s.setBalance(before + interest);
        s.setLastSavedDate(today);
        TASSERT(s.getBalance() > before);
    } else {
        TASSERT(false && "days <= 0");
    }
    TPASS();
}

// 8. Генерация уникальных ID
static void test_GenerateUniqueAccountId() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);
    Customer x("X",40,"x@e","88888888","s");
    for(int id: {111111,222222,333333}) x.addAccount(Account(id,"Checking",0));
    TASSERT(db.addOrUpdateCustomer(x));
    int gen = db.generateUniqueAccountId();
    TASSERT(gen!=111111 && gen!=222222 && gen!=333333);
    TPASS();
}

// 9. Сброс пароля по email
static void test_ResetPassword() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);

    Customer u("U",20,"u@e","99999999","old");
    TASSERT(db.addOrUpdateCustomer(u));
    TASSERT(db.resetSecretWithEmail("99999999","u@e","newpass"));
    Customer out; db.loadCustomer("99999999",out);
    TASSERT(out.getSecretWord()=="newpass");
    TPASS();
}

// 10. Бэкап и защита от пустого JSON
static void test_BackupAndNoEmptyOverwrite() {
    wipeDbArtifacts(TEST_DB);
    DatabaseManager db(TEST_DB);

    Customer c("B",28,"b@e","12121212","s");
    c.addAccount(Account(db.generateUniqueAccountId(),"Checking",1));
    TASSERT(db.addOrUpdateCustomer(c));
    c.setEmail("b2@e");
    TASSERT(db.addOrUpdateCustomer(c));
    TASSERT(fs::exists(TEST_DB+".bak"));

    ifstream in(TEST_DB, ios::binary); in.seekg(0,ios::end);
    TASSERT(in.tellg()>0);
    TPASS();
}

// 11. Битый JSON
static void test_CorruptedJsonGraceful() {
    wipeDbArtifacts(TEST_DB);
    fs::create_directories("data");
    ofstream out(TEST_DB, ios::binary|ios::trunc);
    out << "{ invalid json"; out.close();

    DatabaseManager db(TEST_DB);
    bool ok = db.customerExists("ANY");
    (void)ok;
    TPASS();
}

int main() {
    cout<<"=== BankingSystem TESTS START ===\n";
    test_CreateAndLoadCustomer();
    test_UpdateOnlyOwnBlock();
    test_RemoveCustomer();
    test_AccountCreationRule();
    test_DepositWithdrawEdges();
    test_TransferLikeFlow();
    test_SavingsInterest();
    test_GenerateUniqueAccountId();
    test_ResetPassword();
    test_BackupAndNoEmptyOverwrite();
    test_CorruptedJsonGraceful();
    cout<<"=== ALL TESTS PASSED ===\n";
    wipeDbArtifacts(TEST_DB);
    return 0;
}
