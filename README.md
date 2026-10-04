# Bank Management System

A standalone C++ console-based Bank Management System for the Software Architecture and Design course.

## Requirements covered

The implementation covers the main requirements represented in the SAD/SRS traceability:

- Customer login and PIN authentication
- Three-attempt customer PIN lockout for the current login session
- Account creation with unique account number validation
- Deposit
- Withdrawal with positive-amount and overdraft validation
- Balance inquiry
- Transaction history with timestamps
- Customer profile update
- Admin authentication
- View all customer accounts
- Search account
- Freeze/unfreeze account
- Close account when balance is zero
- Account summary report
- Local text-file persistence

The architecture follows the SAD's layered separation concept: console interaction, authentication/account/transaction/admin/report responsibilities, and file-based persistence.

## Project structure

```text
Bank-Management-System/
├── main.cpp
├── BankSystem.h
├── BankSystem.cpp
├── README.md
├── .gitignore
└── data/
    ├── accounts.txt
    ├── transactions.txt
    └── account_summary_report.txt
```

The report file is generated when the admin selects the report option.

## Compile and run

### Windows (MinGW)

```bash
g++ main.cpp BankSystem.cpp -o bank_management_system.exe
bank_management_system.exe
```

### Linux / macOS

```bash
g++ main.cpp BankSystem.cpp -o bank_management_system
./bank_management_system
```

## Demo admin PIN

```text
9999
```

This fixed PIN is for academic demonstration only and should be replaced by a secure credential mechanism for real deployment.

## Team

- Kruthik Reddy.S — PES2UG24CS241
- Kotipalli Devi Sai Charan — PES2UG24CS231
- Kiran BG — PES2UG24CS227
