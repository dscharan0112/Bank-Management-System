#include "BankSystem.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

BankSystem::BankSystem() {
    loadData();
}

string BankSystem::currentTimestamp() const {
    time_t now = time(nullptr);
    tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    ostringstream out;
    out << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

bool BankSystem::validPin(const string& pin) const {
    if (pin.size() != 4) return false;
    return all_of(pin.begin(), pin.end(), [](unsigned char c) {
        return c >= '0' && c <= '9';
    });
}

bool BankSystem::validAmount(double amount) const {
    return amount > 0.0;
}

void BankSystem::loadData() {
    accounts.clear();
    transactions.clear();

    ifstream accountFile(accountsFile);
    string line;

    while (getline(accountFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string value;
        Account account;

        try {
            getline(ss, value, '|');
            account.accountNumber = stoi(value);
            getline(ss, account.name, '|');
            getline(ss, account.pin, '|');
            getline(ss, value, '|');
            account.balance = stod(value);
            getline(ss, value, '|');
            account.frozen = (value == "1");
            accounts.push_back(account);
        } catch (...) {
            // Ignore malformed records.
        }
    }

    ifstream transactionFile(transactionsFile);
    while (getline(transactionFile, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string value;
        Transaction transaction;

        try {
            getline(ss, value, '|');
            transaction.accountNumber = stoi(value);
            getline(ss, transaction.type, '|');
            getline(ss, value, '|');
            transaction.amount = stod(value);
            getline(ss, value, '|');
            transaction.balanceAfter = stod(value);
            getline(ss, transaction.timestamp, '|');
            transactions.push_back(transaction);
        } catch (...) {
            // Ignore malformed records.
        }
    }
}

void BankSystem::saveAccounts() const {
    ofstream file(accountsFile);
    file << fixed << setprecision(2);

    for (const auto& account : accounts) {
        file << account.accountNumber << '|'
             << account.name << '|'
             << account.pin << '|'
             << account.balance << '|'
             << (account.frozen ? 1 : 0) << '\n';
    }
}

void BankSystem::appendTransaction(const Transaction& transaction) {
    transactions.push_back(transaction);

    ofstream file(transactionsFile, ios::app);
    file << transaction.accountNumber << '|'
         << transaction.type << '|'
         << fixed << setprecision(2)
         << transaction.amount << '|'
         << transaction.balanceAfter << '|'
         << transaction.timestamp << '\n';
}

int BankSystem::findAccount(int accountNumber) const {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].accountNumber == accountNumber)
            return static_cast<int>(i);
    }
    return -1;
}

void BankSystem::createAccount() {
    Account account;

    cout << "\n--- Create Account ---\n";
    cout << "Enter account number: ";
    cin >> account.accountNumber;

    if (findAccount(account.accountNumber) != -1) {
        cout << "Account already exists.\n";
        return;
    }

    cin.ignore();
    cout << "Enter customer name: ";
    getline(cin, account.name);

    cout << "Set 4-digit PIN: ";
    cin >> account.pin;

    if (!validPin(account.pin)) {
        cout << "PIN must contain exactly 4 digits.\n";
        return;
    }

    account.balance = 0.0;
    account.frozen = false;

    accounts.push_back(account);
    saveAccounts();

    cout << "Account created successfully.\n";
}

int BankSystem::customerLogin() {
    int accountNumber;
    string pin;

    cout << "\n--- Customer Login ---\n";
    cout << "Account number: ";
    cin >> accountNumber;

    int index = findAccount(accountNumber);
    if (index == -1) {
        cout << "Account not found.\n";
        return -1;
    }

    for (int attempt = 1; attempt <= 3; ++attempt) {
        cout << "PIN (attempt " << attempt << "/3): ";
        cin >> pin;

        if (accounts[index].pin == pin) {
            if (accounts[index].frozen) {
                cout << "Account is frozen. Contact the administrator.\n";
                return -1;
            }

            cout << "Login successful.\n";
            return index;
        }

        cout << "Invalid PIN.\n";
    }

    cout << "Three failed attempts. Login locked for this session.\n";
    return -1;
}

void BankSystem::deposit(int index) {
    if (accounts[index].frozen) {
        cout << "Transaction rejected: account is frozen.\n";
        return;
    }

    double amount;
    cout << "Enter deposit amount: ";
    cin >> amount;

    if (!validAmount(amount)) {
        cout << "Amount must be positive.\n";
        return;
    }

    accounts[index].balance += amount;
    saveAccounts();

    appendTransaction({
        accounts[index].accountNumber,
        "DEPOSIT",
        amount,
        accounts[index].balance,
        currentTimestamp()
    });

    cout << "Deposit successful. New balance: Rs. "
         << fixed << setprecision(2)
         << accounts[index].balance << '\n';
}

void BankSystem::withdraw(int index) {
    if (accounts[index].frozen) {
        cout << "Transaction rejected: account is frozen.\n";
        return;
    }

    double amount;
    cout << "Enter withdrawal amount: ";
    cin >> amount;

    if (!validAmount(amount)) {
        cout << "Amount must be positive.\n";
        return;
    }

    if (amount > accounts[index].balance) {
        cout << "Insufficient balance. Withdrawal rejected.\n";
        return;
    }

    accounts[index].balance -= amount;
    saveAccounts();

    appendTransaction({
        accounts[index].accountNumber,
        "WITHDRAW",
        amount,
        accounts[index].balance,
        currentTimestamp()
    });

    cout << "Withdrawal successful. New balance: Rs. "
         << fixed << setprecision(2)
         << accounts[index].balance << '\n';
}

void BankSystem::viewTransactions(int accountNumber) const {
    cout << "\n--- Transaction History ---\n";

    bool found = false;
    for (const auto& transaction : transactions) {
        if (transaction.accountNumber == accountNumber) {
            found = true;
            cout << transaction.timestamp << " | "
                 << transaction.type << " | Rs. "
                 << fixed << setprecision(2) << transaction.amount
                 << " | Balance: Rs. " << transaction.balanceAfter << '\n';
        }
    }

    if (!found)
        cout << "No transactions found.\n";
}

void BankSystem::updateProfile(int index) {
    cin.ignore();
    string newName;

    cout << "Current name: " << accounts[index].name << '\n';
    cout << "Enter new name: ";
    getline(cin, newName);

    if (newName.empty()) {
        cout << "Name cannot be empty.\n";
        return;
    }

    accounts[index].name = newName;
    saveAccounts();

    cout << "Profile updated successfully.\n";
}

void BankSystem::customerMenu(int index) {
    int choice;

    do {
        cout << "\n--- Customer Menu ---\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. View Transaction History\n";
        cout << "5. Update Profile\n";
        cout << "6. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Current balance: Rs. "
                     << fixed << setprecision(2)
                     << accounts[index].balance << '\n';
                break;
            case 2:
                deposit(index);
                break;
            case 3:
                withdraw(index);
                break;
            case 4:
                viewTransactions(accounts[index].accountNumber);
                break;
            case 5:
                updateProfile(index);
                break;
            case 6:
                cout << "Logged out.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 6);
}

void BankSystem::viewAllAccounts() const {
    cout << "\n--- All Customer Accounts ---\n";

    if (accounts.empty()) {
        cout << "No accounts available.\n";
        return;
    }

    cout << left << setw(15) << "Account"
         << setw(25) << "Name"
         << setw(15) << "Balance"
         << "Status\n";

    for (const auto& account : accounts) {
        cout << left << setw(15) << account.accountNumber
             << setw(25) << account.name
             << setw(15) << fixed << setprecision(2) << account.balance
             << (account.frozen ? "Frozen" : "Active") << '\n';
    }
}

void BankSystem::searchAccount() const {
    int accountNumber;
    cout << "Enter account number: ";
    cin >> accountNumber;

    int index = findAccount(accountNumber);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    const auto& account = accounts[index];
    cout << "Account: " << account.accountNumber << '\n';
    cout << "Name: " << account.name << '\n';
    cout << "Balance: Rs. " << fixed << setprecision(2)
         << account.balance << '\n';
    cout << "Status: " << (account.frozen ? "Frozen" : "Active") << '\n';
}

void BankSystem::freezeUnfreezeAccount() {
    int accountNumber;
    cout << "Enter account number: ";
    cin >> accountNumber;

    int index = findAccount(accountNumber);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    accounts[index].frozen = !accounts[index].frozen;
    saveAccounts();

    cout << (accounts[index].frozen ? "Account frozen.\n"
                                     : "Account unfrozen.\n");
}

void BankSystem::closeAccount() {
    int accountNumber;
    cout << "Enter account number to close: ";
    cin >> accountNumber;

    int index = findAccount(accountNumber);
    if (index == -1) {
        cout << "Account not found.\n";
        return;
    }

    if (accounts[index].balance != 0.0) {
        cout << "Account cannot be closed while balance is not zero.\n";
        return;
    }

    accounts.erase(accounts.begin() + index);
    saveAccounts();

    cout << "Account closed successfully.\n";
}

void BankSystem::generateReport() const {
    double totalBalance = 0.0;
    int frozenCount = 0;
    int transactionCount = static_cast<int>(transactions.size());

    for (const auto& account : accounts) {
        totalBalance += account.balance;
        if (account.frozen) ++frozenCount;
    }

    ofstream report("data/account_summary_report.txt");
    report << "BANK MANAGEMENT SYSTEM - ACCOUNT SUMMARY REPORT\n";
    report << "================================================\n";
    report << "Total accounts: " << accounts.size() << '\n';
    report << "Frozen accounts: " << frozenCount << '\n';
    report << "Total balance: Rs. " << fixed << setprecision(2)
           << totalBalance << '\n';
    report << "Total logged transactions: " << transactionCount << '\n';

    cout << "\n--- Account Summary Report ---\n";
    cout << "Total accounts: " << accounts.size() << '\n';
    cout << "Frozen accounts: " << frozenCount << '\n';
    cout << "Total balance: Rs. " << fixed << setprecision(2)
         << totalBalance << '\n';
    cout << "Total logged transactions: " << transactionCount << '\n';
    cout << "Report saved to data/account_summary_report.txt\n";
}

void BankSystem::adminLogin() {
    string pin;

    cout << "\n--- Admin Login ---\n";
    cout << "Admin PIN: ";
    cin >> pin;

    if (pin != adminPin) {
        cout << "Invalid admin PIN.\n";
        return;
    }

    cout << "Admin login successful.\n";
    adminMenu();
}

void BankSystem::adminMenu() {
    int choice;

    do {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. View All Accounts\n";
        cout << "2. Search Account\n";
        cout << "3. Freeze/Unfreeze Account\n";
        cout << "4. Close Account\n";
        cout << "5. Generate Account Summary Report\n";
        cout << "6. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                viewAllAccounts();
                break;
            case 2:
                searchAccount();
                break;
            case 3:
                freezeUnfreezeAccount();
                break;
            case 4:
                closeAccount();
                break;
            case 5:
                generateReport();
                break;
            case 6:
                cout << "Admin logged out.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 6);
}

void BankSystem::run() {
    int choice;

    do {
        cout << "\n====================================\n";
        cout << "       BANK MANAGEMENT SYSTEM\n";
        cout << "====================================\n";
        cout << "1. Create Account\n";
        cout << "2. Customer Login\n";
        cout << "3. Admin Login\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                createAccount();
                break;

            case 2: {
                int index = customerLogin();
                if (index != -1)
                    customerMenu(index);
                break;
            }

            case 3:
                adminLogin();
                break;

            case 4:
                cout << "Thank you for using the Bank Management System.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 4);
}