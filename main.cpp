#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

using namespace std;

struct Account {
    int accountNumber{};
    string name;
    string pin;
    double balance{};
    bool frozen{false};
};

const string DATA_FILE = "data/accounts.txt";

vector<Account> accounts;

void loadAccounts() {
    accounts.clear();
    ifstream file(DATA_FILE);
    if (!file) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string value;
        Account a;

        getline(ss, value, '|'); a.accountNumber = stoi(value);
        getline(ss, a.name, '|');
        getline(ss, a.pin, '|');
        getline(ss, value, '|'); a.balance = stod(value);
        getline(ss, value, '|'); a.frozen = (value == "1");

        accounts.push_back(a);
    }
}

void saveAccounts() {
    ofstream file(DATA_FILE);
    for (const auto& a : accounts) {
        file << a.accountNumber << '|'
             << a.name << '|'
             << a.pin << '|'
             << fixed << setprecision(2) << a.balance << '|'
             << (a.frozen ? 1 : 0) << '\n';
    }
}

int findAccount(int accountNumber) {
    for (size_t i = 0; i < accounts.size(); ++i) {
        if (accounts[i].accountNumber == accountNumber)
            return static_cast<int>(i);
    }
    return -1;
}

void createAccount() {
    Account a;

    cout << "\n--- Create Account ---\n";
    cout << "Enter account number: ";
    cin >> a.accountNumber;

    if (findAccount(a.accountNumber) != -1) {
        cout << "Account already exists.\n";
        return;
    }

    cin.ignore();
    cout << "Enter customer name: ";
    getline(cin, a.name);

    cout << "Set 4-digit PIN: ";
    cin >> a.pin;

    if (a.pin.length() != 4) {
        cout << "PIN must contain exactly 4 digits.\n";
        return;
    }

    for (char c : a.pin) {
        if (!isdigit(static_cast<unsigned char>(c))) {
            cout << "PIN must contain digits only.\n";
            return;
        }
    }

    a.balance = 0.0;
    a.frozen = false;
    accounts.push_back(a);
    saveAccounts();

    cout << "Account created successfully.\n";
}

int customerLogin() {
    int accountNumber;
    string pin;

    cout << "\n--- Customer Login ---\n";
    cout << "Account number: ";
    cin >> accountNumber;
    cout << "PIN: ";
    cin >> pin;

    int index = findAccount(accountNumber);

    if (index == -1) {
        cout << "Account not found.\n";
        return -1;
    }

    if (accounts[index].frozen) {
        cout << "Account is frozen. Contact the administrator.\n";
        return -1;
    }

    if (accounts[index].pin != pin) {
        cout << "Invalid PIN.\n";
        return -1;
    }

    cout << "Login successful.\n";
    return index;
}

void deposit(int index) {
    double amount;
    cout << "Enter deposit amount: ";
    cin >> amount;

    if (amount <= 0) {
        cout << "Amount must be positive.\n";
        return;
    }

    accounts[index].balance += amount;
    saveAccounts();

    cout << "Deposit successful. New balance: Rs. "
         << fixed << setprecision(2) << accounts[index].balance << '\n';
}

void withdraw(int index) {
    double amount;
    cout << "Enter withdrawal amount: ";
    cin >> amount;

    if (amount <= 0) {
        cout << "Amount must be positive.\n";
        return;
    }

    if (amount > accounts[index].balance) {
        cout << "Insufficient balance.\n";
        return;
    }

    accounts[index].balance -= amount;
    saveAccounts();

    cout << "Withdrawal successful. New balance: Rs. "
         << fixed << setprecision(2) << accounts[index].balance << '\n';
}

void customerMenu(int index) {
    int choice;

    do {
        cout << "\n--- Customer Menu ---\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit\n";
        cout << "3. Withdraw\n";
        cout << "4. View Profile\n";
        cout << "5. Logout\n";
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
                cout << "Account Number: " << accounts[index].accountNumber << '\n';
                cout << "Name: " << accounts[index].name << '\n';
                break;
            case 5:
                cout << "Logged out.\n";
                break;
            default:
                cout << "Invalid choice.\n";
        }
    } while (choice != 5);
}

void adminMenu() {
    const string ADMIN_PIN = "9999";
    string pin;

    cout << "\n--- Admin Login ---\n";
    cout << "Admin PIN: ";
    cin >> pin;

    if (pin != ADMIN_PIN) {
        cout << "Invalid admin PIN.\n";
        return;
    }

    int choice;
    do {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. View All Accounts\n";
        cout << "2. Freeze/Unfreeze Account\n";
        cout << "3. Close Account\n";
        cout << "4. Account Summary Report\n";
        cout << "5. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1) {
            if (accounts.empty()) {
                cout << "No accounts available.\n";
            } else {
                cout << "\nAccount List\n";
                cout << left << setw(15) << "Account"
                     << setw(25) << "Name"
                     << setw(15) << "Balance"
                     << "Status\n";

                for (const auto& a : accounts) {
                    cout << left << setw(15) << a.accountNumber
                         << setw(25) << a.name
                         << setw(15) << fixed << setprecision(2) << a.balance
                         << (a.frozen ? "Frozen" : "Active") << '\n';
                }
            }
        }
        else if (choice == 2) {
            int accountNumber;
            cout << "Enter account number: ";
            cin >> accountNumber;

            int index = findAccount(accountNumber);
            if (index == -1) {
                cout << "Account not found.\n";
            } else {
                accounts[index].frozen = !accounts[index].frozen;
                saveAccounts();
                cout << (accounts[index].frozen ? "Account frozen.\n"
                                                  : "Account unfrozen.\n");
            }
        }
        else if (choice == 3) {
            int accountNumber;
            cout << "Enter account number to close: ";
            cin >> accountNumber;

            int index = findAccount(accountNumber);
            if (index == -1) {
                cout << "Account not found.\n";
            } else {
                accounts.erase(accounts.begin() + index);
                saveAccounts();
                cout << "Account closed successfully.\n";
            }
        }
        else if (choice == 4) {
            double total = 0.0;
            for (const auto& a : accounts)
                total += a.balance;

            cout << "\n--- Account Summary Report ---\n";
            cout << "Total accounts: " << accounts.size() << '\n';
            cout << "Total balance: Rs. "
                 << fixed << setprecision(2) << total << '\n';
        }
        else if (choice == 5) {
            cout << "Admin logged out.\n";
        }
        else {
            cout << "Invalid choice.\n";
        }

    } while (choice != 5);
}

int main() {
    loadAccounts();

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
                adminMenu();
                break;

            case 4:
                cout << "Thank you for using the Bank Management System.\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while (choice != 4);

    return 0;
}