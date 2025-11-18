/*
FullName: NGUYEN XUAN TOAN
ID Student: 24110138
Purpose: Bank Account Management System
Date: Sep 17th, 2025
*/
#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using string_list = vector<string>;

static ll NEXT_ACCOUNT_ID = 1001;

enum class AccountType { BASE, CHECKING, SAVINGS };

struct Transaction {
    time_t t;
    string desc;
    double amount; // positive for deposits, negative for withdrawals
    Transaction(double amt = 0.0, const string &d = "") : t(time(nullptr)), desc(d), amount(amt) {}
    string toString() const {
        char buf[64];
        struct tm tm;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        string s = string(buf) + " | ";
        s += (amount >= 0 ? "+ " : "- ");
        s += to_string(fabs(amount));
        s += " | ";
        s += desc;
        return s;
    }
};

class Account {
protected:
    ll account_id;
    string owner_name;
    double balance;
    vector<Transaction> transactions;
    AccountType acct_type;
public:
    Account() : account_id(NEXT_ACCOUNT_ID++), owner_name(""), balance(0.0), acct_type(AccountType::BASE) {}
    Account(const string &owner, double bal = 0.0) : account_id(NEXT_ACCOUNT_ID++), owner_name(owner), balance(bal), acct_type(AccountType::BASE) {}
    virtual ~Account() = default;

    ll getId() const { return account_id; }
    const string& getOwner() const { return owner_name; }
    double getBalance() const { return balance; }
    AccountType getType() const { return acct_type; }

    virtual bool deposit(double amount) {
        if (amount <= 0) return false;
        balance += amount;
        transactions.emplace_back(amount, "Deposit");
        return true;
    }
    virtual bool withdraw(double amount) {
        if (amount <= 0) return false;
        if (amount > balance) return false;
        balance -= amount;
        transactions.emplace_back(-amount, "Withdrawal");
        return true;
    }

    virtual string typeName() const { return "Account"; }

    void addTransaction(const Transaction &t) {
        transactions.push_back(t);
    }

    const vector<Transaction>& getTransactions() const {
        return transactions;
    }

    // operator overloading: compare based on balance
    bool operator<(const Account &other) const {
        return balance < other.balance;
    }

    // operator overloading: += to deposit
    Account& operator+=(double amount) {
        deposit(amount);
        return *this;
    }

    friend ostream& operator<<(ostream &os, const Account &a) {
        os << "[" << a.typeName() << "] ID: " << a.account_id
           << " | Owner: " << a.owner_name
           << " | Balance: " << fixed << setprecision(2) << a.balance;
        return os;
    }
};

class SavingsAccount : public Account {
    double interest_rate; // yearly interest rate, e.g. 0.03 for 3%
public:
    SavingsAccount(const string &owner, double bal = 0.0, double rate = 0.02)
        : Account(owner, bal), interest_rate(rate) { acct_type = AccountType::SAVINGS; }

    bool applyInterest(double months = 1.0) {
        if (months <= 0) return false;
        double monthly_rate = interest_rate / 12.0;
        double interest = balance * monthly_rate * months;
        balance += interest;
        transactions.emplace_back(interest, "Interest");
        return true;
    }

    virtual string typeName() const override { return "Savings"; }
};

class CheckingAccount : public Account {
    double overdraft_limit; // positive number; allow negative balance up to -overdraft_limit
public:
    CheckingAccount(const string &owner, double bal = 0.0, double overdraft = 0.0)
        : Account(owner, bal), overdraft_limit(overdraft) { acct_type = AccountType::CHECKING; }

    virtual bool withdraw(double amount) override {
        if (amount <= 0) return false;
        if (balance - amount < -overdraft_limit) return false;
        balance -= amount;
        transactions.emplace_back(-amount, "Withdrawal");
        return true;
    }

    virtual string typeName() const override { return "Checking"; }
};

class Bank {
    // Use unique_ptr to store mixed Account types
    unordered_map<ll, unique_ptr<Account>> accounts;
public:
    Bank() = default;

    Account* createAccount(const string &owner, AccountType type = AccountType::BASE, double initial = 0.0) {
        Account* a = nullptr;
        if (type == AccountType::SAVINGS) {
            a = new SavingsAccount(owner, initial);
        } else if (type == AccountType::CHECKING) {
            a = new CheckingAccount(owner, initial, 500.0); // example overdraft
        } else {
            a = new Account(owner, initial);
        }
        ll id = a->getId();
        accounts[id] = unique_ptr<Account>(a);
        return accounts[id].get();
    }

    bool deleteAccount(ll id) {
        auto it = accounts.find(id);
        if (it == accounts.end()) return false;
        accounts.erase(it);
        return true;
    }

    Account* findAccount(ll id) {
        auto it = accounts.find(id);
        if (it == accounts.end()) return nullptr;
        return it->second.get();
    }

    vector<Account*> listAccounts() {
        vector<Account*> out;
        out.reserve(accounts.size());
        for (auto &p : accounts) out.push_back(p.second.get());
        // sort by id for display consistency
        sort(out.begin(), out.end(), [](Account* a, Account* b){ return a->getId() < b->getId(); });
        return out;
    }

    bool transfer(ll from_id, ll to_id, double amount) {
        if (amount <= 0) return false;
        Account* A = findAccount(from_id);
        Account* B = findAccount(to_id);
        if (!A || !B) return false;
        if (!A->withdraw(amount)) return false;
        B->deposit(amount);
        A->addTransaction(Transaction(-amount, "Transfer to " + to_string(to_id)));
        B->addTransaction(Transaction(amount, "Transfer from " + to_string(from_id)));
        return true;
    }
};

// Utility functions
static void printHeader() {
    cout << "=========================\n";
    cout << "Bank Account Management\n";
    cout << "=========================\n";
}

static int getInt() {
    int x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return x;
}

static double getDouble() {
    double x;
    while (!(cin >> x)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return x;
}

static string getLineTrimmed() {
    string s;
    getline(cin, s);
    // trim both ends
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

void interactiveDemo() {
    Bank bank;
    // sample accounts
    bank.createAccount("Alice Nguyen", AccountType::SAVINGS, 1000.0);
    bank.createAccount("Bob Tran", AccountType::CHECKING, 200.0);
    bank.createAccount("Clara Le", AccountType::BASE, 50.0);

    while (true) {
        printHeader();
        cout << "1. List accounts\n";
        cout << "2. Create account\n";
        cout << "3. Delete account\n";
        cout << "4. Deposit\n";
        cout << "5. Withdraw\n";
        cout << "6. Transfer\n";
        cout << "7. Show account details & transactions\n";
        cout << "8. Apply interest to a savings account\n";
        cout << "9. Compare two accounts by balance\n";
        cout << "0. Exit\n";
        cout << "Choose an option: ";
        int opt = getInt();

        if (opt == 0) {
            cout << "Exiting. Goodbye!\n";
            break;
        }

        if (opt == 1) {
            auto list = bank.listAccounts();
            cout << "Accounts (" << list.size() << "):\n";
            for (auto a : list) {
                cout << *a << "\n";
            }
            cout << "Press Enter to continue...";
            cin.get();
        } else if (opt == 2) {
            cout << "Owner name: ";
            string name = getLineTrimmed();
            cout << "Account type (0=Base,1=Checking,2=Savings): ";
            int t = getInt();
            cout << "Initial deposit: ";
            double init = getDouble();
            AccountType at = AccountType::BASE;
            if (t == 1) at = AccountType::CHECKING;
            else if (t == 2) at = AccountType::SAVINGS;
            Account* a = bank.createAccount(name, at, init);
            cout << "Created: " << *a << "\n";
            cout << "Press Enter to continue...";
            cin.get();
        } else if (opt == 3) {
            cout << "Account ID to delete: ";
            ll id;
            cin >> id;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (bank.deleteAccount(id)) cout << "Deleted account " << id << "\n";
            else cout << "Account not found.\n";
            cout << "Press Enter to continue...";
            cin.get();
        } else if (opt == 4) {
            cout << "Account ID to deposit: ";
            ll id; cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Amount: "; double amt = getDouble();
            Account* a = bank.findAccount(id);
            if (!a) cout << "Account not found.\n";
            else if (a->deposit(amt)) cout << "Deposit successful. New balance: " << a->getBalance() << "\n";
            else cout << "Deposit failed.\n";
            cout << "Press Enter to continue..."; cin.get();
        } else if (opt == 5) {
            cout << "Account ID to withdraw: ";
            ll id; cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Amount: "; double amt = getDouble();
            Account* a = bank.findAccount(id);
            if (!a) cout << "Account not found.\n";
            else if (a->withdraw(amt)) cout << "Withdraw successful. New balance: " << a->getBalance() << "\n";
            else cout << "Withdraw failed (insufficient funds or limit).\n";
            cout << "Press Enter to continue..."; cin.get();
        } else if (opt == 6) {
            cout << "From account ID: "; ll from; cin >> from; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "To account ID: "; ll to; cin >> to; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Amount: "; double amt = getDouble();
            if (bank.transfer(from, to, amt)) cout << "Transfer successful.\n";
            else cout << "Transfer failed.\n";
            cout << "Press Enter to continue..."; cin.get();
        } else if (opt == 7) {
            cout << "Account ID: "; ll id; cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Account* a = bank.findAccount(id);
            if (!a) {
                cout << "Account not found.\n";
            } else {
                cout << *a << "\n";
                cout << "Transactions: \n";
                auto tx = a->getTransactions();
                for (auto &t : tx) {
                    cout << "  - " << t.toString() << "\n";
                }
            }
            cout << "Press Enter to continue..."; cin.get();
        } else if (opt == 8) {
            cout << "Savings Account ID: "; ll id; cin >> id; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Months to apply interest (e.g., 1): "; double m = getDouble();
            Account* a = bank.findAccount(id);
            if (!a) cout << "Account not found.\n";
            else {
                SavingsAccount* sa = dynamic_cast<SavingsAccount*>(a);
                if (!sa) cout << "Not a savings account.\n";
                else {
                    if (sa->applyInterest(m)) cout << "Interest applied. New balance: " << sa->getBalance() << "\n";
                    else cout << "Failed to apply interest.\n";
                }
            }
            cout << "Press Enter to continue..."; cin.get();
        } else if (opt == 9) {
            cout << "First account ID: "; ll a1; cin >> a1; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Second account ID: "; ll a2; cin >> a2; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            Account* A = bank.findAccount(a1);
            Account* B = bank.findAccount(a2);
            if (!A || !B) cout << "One or both accounts not found.\n";
            else {
                if (*A < *B) cout << a1 << " has less balance than " << a2 << "\n";
                else if (*B < *A) cout << a2 << " has less balance than " << a1 << "\n";
                else cout << "Balances are equal.\n";
            }
            cout << "Press Enter to continue..."; cin.get();
        } else {
            cout << "Unknown option.\n";
            cout << "Press Enter to continue..."; cin.get();
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    interactiveDemo();
    return 0;
}
