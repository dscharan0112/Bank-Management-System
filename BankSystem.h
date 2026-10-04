#ifndef BANK_SYSTEM_H
#define BANK_SYSTEM_H

#include <string>
#include <vector>

struct Account {
    int accountNumber{};
    std::string name;
    std::string pin;
    double balance{};
    bool frozen{false};
};

struct Transaction {
    int accountNumber{};
    std::string type;
    double amount{};
    double balanceAfter{};
    std::string timestamp;
};

class BankSystem {
private:
    std::vector<Account> accounts;
    std::vector<Transaction> transactions;

    const std::string accountsFile = "data/accounts.txt";
    const std::string transactionsFile = "data/transactions.txt";
    const std::string adminPin = "9999";

    void loadData();
    void saveAccounts() const;
    void appendTransaction(const Transaction& transaction);
    int findAccount(int accountNumber) const;

    void createAccount();
    int customerLogin();
    void customerMenu(int index);

    void deposit(int index);
    void withdraw(int index);
    void viewTransactions(int accountNumber) const;
    void updateProfile(int index);

    void adminLogin();
    void adminMenu();
    void viewAllAccounts() const;
    void searchAccount() const;
    void freezeUnfreezeAccount();
    void closeAccount();
    void generateReport() const;

    std::string currentTimestamp() const;
    bool validPin(const std::string& pin) const;
    bool validAmount(double amount) const;

public:
    BankSystem();
    void run();
};

#endif