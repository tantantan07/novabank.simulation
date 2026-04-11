<div align="center">

# Nova Bank

**A modular, command-line banking system built in C**

![Language](https://img.shields.io/badge/Language-C-blue?style=flat-square&logo=c)
![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20macOS-lightgrey?style=flat-square)
![License](https://img.shields.io/badge/License-Educational-green?style=flat-square)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=flat-square)
![OpenSSL](https://img.shields.io/badge/Dependency-OpenSSL-red?style=flat-square&logo=openssl)

Nova Bank simulates core banking operations — account management, transactions, credit handling, and secure authentication — with a strong focus on data integrity, structured design, and precision in financial calculations.

</div>

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [Project Structure](#-project-structure)
- [Design Highlights](#-design-highlights)
- [Getting Started](#-getting-started)
- [Compilation & Execution](#-compilation--execution)
- [Debug Utility](#-debug-utility)
- [Concepts Demonstrated](#-concepts-demonstrated)
- [License](#-license)

---

##  Overview

Nova Bank is designed to reflect real-world banking concepts using low-level programming in C. It emphasizes:

- **Modular architecture** with clear separation of concerns across dedicated source files
- **Secure credential handling** using cryptographic hashing with salted SHA-256
- **Accurate financial computation** using integer-based currency stored in paise
- **Persistent storage** via atomic binary file writes
- **Polished terminal interface** with ANSI colour-coded output and structured menus

---

##  Features

###  Account Management
- Create accounts with a full name and a user-set 4-digit PIN
- PIN confirmation required at creation; up to 3 attempts allowed
- Secure login via account number and PIN
- Account automatically **locked** after 3 consecutive failed login attempts

###  Financial Operations
- Deposit and withdraw funds with real-time balance feedback
- Large withdrawal warning triggered for amounts exceeding **Rs. 20,000**
- Transfer money between accounts with receiver name preview and a confirmation prompt before committing
- Debit card payments verified against the account's 16-digit card number
- Credit card payments checked against available credit in real time

###  Credit System
- Default credit limit of **Rs. 50,000** per account
- Credit usage tracked separately from account balance
- Transactions declined automatically if they exceed available credit
- Partial credit bill payments supported — pay any amount up to the outstanding balance

### Interest Calculation

Interest is applied automatically on login, once every 30 days, using a three-tier rate system:

| Balance Range | Interest Rate |
|---|---|
| Below Rs. 10,000 | 1% |
| Rs. 10,000 – Rs. 50,000 | 2% |
| Above Rs. 50,000 | 3% |

###  Transaction Tracking
- Every operation is logged with a **timestamp** (`DD Mon YYYY | HH:MM`) and a description
- Full transaction history viewable in a formatted, colour-coded table
- Mini statement shows the **last 5 transactions** at a glance
- Deposits and credits shown in green; withdrawals and payments in red

###  Data Persistence
- All data stored in `bank.dat` as a binary file
- Writes go to `bank.tmp` first, then atomically renamed to `bank.dat`
- Corrupt or out-of-range account counts detected on load; system starts fresh rather than crashing

###  Security
- PINs hashed with **SHA-256 + a 16-byte cryptographically random salt** (via OpenSSL)
- No plain-text credentials written to disk at any point
- Card numbers generated using `RAND_bytes` from OpenSSL — cryptographically secure
- Masked PIN entry using raw terminal mode (`termios`) — input is never echoed

### ⌨️ Input Handling
- `read_int` and `read_double` use `strtol`/`strtod` with strict validation — no `scanf` abuse for user amounts
- All financial inputs reject zero, negative, and non-numeric values
- PIN entry supports backspace correction before submission

---

##  Project Structure

```
nova-bank/
├── include/
│   ├── bank.h          # Core structs, constants, and global state
│   ├── account.h       # Account and banking operation declarations
│   ├── transaction.h   # Transaction logging declarations
│   ├── file.h          # Save/load declarations
│   ├── utils.h         # Input, hashing, and number generation
│   └── ui.h            # ANSI UI components and print helpers
├── src/
│   ├── main.c          # Entry point; main menu loop
│   ├── account.c       # All banking operations
│   ├── transaction.c   # Transaction logging and display
│   ├── file.c          # Binary file persistence
│   └── utils.c         # Input parsing, PIN masking, SHA-256 hashing
├── debug.c             # Standalone binary database inspector
├── bank.dat            # Binary data file (auto-generated on first run)
└── README.md
```

---

##  Design Highlights

### Precision-Safe Currency Handling

All monetary values are stored as **integers in paise** — never as floats:

```c
// 1 Rs = 100 paise
static inline long long rs_to_paise(double rs) { return (long long)(rs * 100 + 0.5); }
static inline double paise_to_rs(long long p)  { return p / 100.0; }
```

This eliminates floating-point rounding errors entirely, which is standard practice in financial systems.

---

### Atomic File Saving

Data is written to a temporary file first, then renamed in one operation:

```
bank.tmp  →  bank.dat
```

This means a crash mid-write will never corrupt the existing data file — the old `bank.dat` remains intact until the new one is fully ready.

---

### PIN Security (SHA-256 + Salt)

Each PIN is combined with a unique 16-byte random salt and hashed using SHA-256 before storage:

```c
void hashPIN(int pin, unsigned char *salt, unsigned char *output) {
    unsigned char input[20];        // 16-byte salt + 4-byte pin
    memcpy(input, salt, 16);
    memcpy(input + 16, &pin, sizeof(int));
    SHA256(input, 16 + sizeof(int), output);
}
```

Even if `bank.dat` is accessed directly, no PIN can be recovered from it.

---

### ANSI Terminal UI

The interface uses ANSI escape codes defined in `ui.h` for colour, bold, and dim text — giving the CLI a clean, structured appearance with colour-coded feedback (green for success, red for errors, yellow for warnings).

---

##  Getting Started

### Prerequisites

- GCC or any compatible C compiler
- OpenSSL development libraries

**Ubuntu / Debian:**
```bash
sudo apt install gcc libssl-dev
```

**macOS (Homebrew):**
```bash
brew install openssl
```

---

##  Compilation & Execution

### Compile

```bash
gcc src/*.c -o bank -lssl -lcrypto
```

### Run

```bash
./bank
```

`bank.dat` is created automatically on the first account registration.

---

##  Debug Utility

`debug.c` is a standalone tool that reads `bank.dat` and prints all stored account data in a human-readable format — including name, account number, balance, salt, and PIN hash.

### Compile

```bash
gcc debug.c -o debug
```

### Run

```bash
./debug
```

> Useful for verifying data integrity and confirming that credentials are stored only as hashes — never in plain text.

---

##  Concepts Demonstrated

| Concept | Implementation |
|---|---|
| Modular programming | Distinct `.c` / `.h` files per domain |
| Binary file I/O | Struct serialisation with atomic writes |
| Cryptographic hashing | SHA-256 + 16-byte salt via OpenSSL |
| Secure random generation | `RAND_bytes` for card numbers and salts |
| Terminal input handling | `termios` raw mode for masked PIN entry |
| Integer currency arithmetic | All values in paise; no floating-point money |
| Time-based logic | `difftime` for 30-day interest period checks |
| Defensive input parsing | `strtol` / `strtod` with strict error checking |

---

##  License

This project is intended for **educational purposes** and may be freely modified or extended.

---

<div align="center">
  <sub>Built with C · Secured with OpenSSL · Designed for learning</sub>
</div>
