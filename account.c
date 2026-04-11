#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "bank.h"
#include "utils.h"
#include "transaction.h"
#include "file.h"
#include "account.h"
#include "ui.h"
#include <openssl/rand.h>

/* ═══════════════════════════════════════════════════════════
   INTERNAL HELPERS
   ═══════════════════════════════════════════════════════════ */

/* Format paise into a "Rs. X.XX" string */
static void fmt(char *buf, int bufsz, long long paise) {
    snprintf(buf, bufsz, "Rs. %.2f", paise / 100.0);
}

/* Read a rupee amount from user → return paise, or -1 on bad input */
static long long read_amount(void) {
    double rs;
    if (!read_double(&rs) || rs <= 0) return -1;
    return rs_to_paise(rs);
}

/* ═══════════════════════════════════════════════════════════
   INTEREST
   ═══════════════════════════════════════════════════════════ */
void processInterest(int i) {
    time_t now = time(NULL);
    if (bank[i].lastInterest != 0 &&
        difftime(now, bank[i].lastInterest) < INTEREST_PERIOD_SECS)
        return;

    float rate;
    if      (bank[i].balance < INTEREST_TIER_1) rate = INTEREST_RATE_1;
    else if (bank[i].balance < INTEREST_TIER_2) rate = INTEREST_RATE_2;
    else                                         rate = INTEREST_RATE_3;

    long long interest = (long long)(bank[i].balance * rate);
    if (interest == 0) { bank[i].lastInterest = now; return; }

    bank[i].balance     += interest;
    bank[i].lastInterest = now;

    char msg[TX_MSG_LEN];
    snprintf(msg, sizeof(msg), "Interest credited: Rs. %.2f", interest / 100.0);
    addTransaction(i, msg);

    char notice[TX_MSG_LEN];
    snprintf(notice, sizeof(notice),
             "Monthly interest of Rs. %.2f has been credited!", interest / 100.0);
    print_info(notice);
}

/* ═══════════════════════════════════════════════════════════
   FIND ACCOUNT  (linear scan — fine for MAX=100)
   ═══════════════════════════════════════════════════════════ */
int find(long long n) {
    for (int i = 0; i < accCount; i++)
        if (bank[i].accNo == n) return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
   DEPOSIT
   ═══════════════════════════════════════════════════════════ */
void deposit(int i) {
    print_section("DEPOSIT MONEY");
    printf("\n  Enter deposit amount (Rs.): ");

    long long amt = read_amount();
    if (amt <= 0) { print_error("Invalid amount."); return; }

    bank[i].balance += amt;

    char msg[TX_MSG_LEN], notice[TX_MSG_LEN];
    snprintf(msg,    sizeof(msg),    "Deposited: Rs. %.2f", amt / 100.0);
    snprintf(notice, sizeof(notice),
             "Rs. %.2f deposited. New Balance: Rs. %.2f",
             amt / 100.0, bank[i].balance / 100.0);
    addTransaction(i, msg);
    print_success(notice);
}

/* ═══════════════════════════════════════════════════════════
   WITHDRAW
   ═══════════════════════════════════════════════════════════ */
void withdraw(int i) {
    print_section("WITHDRAW MONEY");
    printf("\n  Current Balance    : " GREEN "Rs. %.2f\n" RESET,
           bank[i].balance / 100.0);
    printf("  Enter amount (Rs.) : ");

    long long amt = read_amount();
    if (amt <= 0)               { print_error("Invalid amount.");               return; }
    if (amt > bank[i].balance)  { print_error("Insufficient balance.");         return; }
    if (amt > LARGE_TX_THRESHOLD)
        print_warning("Large transaction! Amount exceeds Rs. 20,000.");

    bank[i].balance -= amt;

    char msg[TX_MSG_LEN], notice[TX_MSG_LEN];
    snprintf(msg,    sizeof(msg),    "Withdrew: Rs. %.2f", amt / 100.0);
    snprintf(notice, sizeof(notice),
             "Rs. %.2f withdrawn. Remaining Balance: Rs. %.2f",
             amt / 100.0, bank[i].balance / 100.0);
    addTransaction(i, msg);
    print_success(notice);
}

/* ═══════════════════════════════════════════════════════════
   TRANSFER
   Shows receiver name before confirming amount,
   asks for confirmation before committing.
   ═══════════════════════════════════════════════════════════ */
void transfer(int i) {
    print_section("TRANSFER MONEY");

    char tbuf[32];
    printf("\n  Enter Receiver Account No  : ");
    fflush(stdout);
    if (!fgets(tbuf, sizeof(tbuf), stdin)) {
        print_error("Input error.");
        return;
    }

    long long target = atoll(tbuf);
    if (target <= 0) {
        print_error("Invalid account number. Must be a positive number.");
        return;
    }

    int j = find(target);
    if (j == -1) { print_error("Account not found. Please verify the account number."); return; }
    if (j == i)  { print_error("Cannot transfer to your own account."); return; }

    printf("  Sending to         : " BOLD CYAN "%s\n" RESET, bank[j].name);
    printf("  Enter amount (Rs.) : ");

    long long amt = read_amount();
    if (amt <= 0)              { print_error("Invalid amount.");        return; }
    if (amt > bank[i].balance) { print_error("Insufficient balance."); return; }

    /* confirmation step */
    printf("\n" YELLOW "  Confirm: Send Rs. %.2f to %s? [y/N]: " RESET,
           amt / 100.0, bank[j].name);
    fflush(stdout);
    char confirm[4];
    if (!fgets(confirm, sizeof(confirm), stdin)) {
        print_error("Input error.");
        return;
    }
    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        print_info("Transfer cancelled.");
        return;
    }

    bank[i].balance -= amt;
    bank[j].balance += amt;

    char msg1[TX_MSG_LEN], msg2[TX_MSG_LEN], notice[TX_MSG_LEN];
    snprintf(msg1,   sizeof(msg1),   "Sent Rs. %.2f to Acc %lld (%s)",      amt / 100.0, target,         bank[j].name);
    snprintf(msg2,   sizeof(msg2),   "Received Rs. %.2f from Acc %lld (%s)", amt / 100.0, bank[i].accNo, bank[i].name);
    snprintf(notice, sizeof(notice), "Rs. %.2f transferred to %s successfully!", amt / 100.0, bank[j].name);

    addTransaction(i, msg1);
    addTransaction(j, msg2);
    print_success(notice);
}
/* ═══════════════════════════════════════════════════════════
   DEBIT CARD SWIPE
   ═══════════════════════════════════════════════════════════ */
void swipeDebit(int i) {
    print_section("DEBIT CARD PAYMENT");

    long long card;
    printf("\n  Enter 16-digit Debit Card No : ");
    scanf("%lld", &card);
    /* flush leftover newline */
    int ch; while ((ch = getchar()) != '\n' && ch != EOF);

    if (card != bank[i].debitCard) { print_error("Invalid debit card number."); return; }

    printf("  Enter payment amount (Rs.)   : ");
    long long amt = read_amount();
    if (amt <= 0)              { print_error("Invalid amount.");              return; }
    if (amt > bank[i].balance) { print_error("Insufficient balance. Declined."); return; }

    bank[i].balance -= amt;

    char msg[TX_MSG_LEN], notice[TX_MSG_LEN];
    snprintf(msg,    sizeof(msg),    "Debit card payment: Rs. %.2f", amt / 100.0);
    snprintf(notice, sizeof(notice), "Payment of Rs. %.2f via Debit Card successful.",
             amt / 100.0);
    addTransaction(i, msg);
    print_success(notice);
}

/* ═══════════════════════════════════════════════════════════
   CREDIT CARD SWIPE
   ═══════════════════════════════════════════════════════════ */
void swipeCredit(int i) {
    print_section("CREDIT CARD PAYMENT");

    long long avail = bank[i].creditLimit - bank[i].creditUsed;
    printf("\n  Credit Available              : " GREEN "Rs. %.2f\n" RESET,
           avail / 100.0);

    long long card;
    printf("  Enter 16-digit Credit Card No : ");
    scanf("%lld", &card);
    int ch; while ((ch = getchar()) != '\n' && ch != EOF);

    if (card != bank[i].creditCard) { print_error("Invalid credit card number."); return; }

    printf("  Enter payment amount (Rs.)    : ");
    long long amt = read_amount();
    if (amt <= 0)        { print_error("Invalid amount.");              return; }
    if (amt > avail)     { print_error("Credit limit exceeded. Declined."); return; }

    bank[i].creditUsed += amt;

    char msg[TX_MSG_LEN], notice[TX_MSG_LEN], warn[TX_MSG_LEN];
    snprintf(msg,    sizeof(msg),    "Credit card payment: Rs. %.2f", amt / 100.0);
    snprintf(notice, sizeof(notice), "Payment of Rs. %.2f via Credit Card successful.",
             amt / 100.0);
    snprintf(warn,   sizeof(warn),
             "Outstanding credit bill is now Rs. %.2f",
             bank[i].creditUsed / 100.0);
    addTransaction(i, msg);
    print_success(notice);
    print_warning(warn);
}

/* ═══════════════════════════════════════════════════════════
   PAY CREDIT BILL
   New: allows partial payment, shows exactly how much is left.
   ═══════════════════════════════════════════════════════════ */
void payCredit(int i) {
    print_section("PAY CREDIT BILL");

    if (bank[i].creditUsed == 0) {
        print_info("No outstanding credit bill. You're all clear!");
        return;
    }

    printf("\n  Outstanding Bill  : " RED    "Rs. %.2f\n" RESET, bank[i].creditUsed / 100.0);
    printf("  Account Balance   : " GREEN  "Rs. %.2f\n" RESET, bank[i].balance    / 100.0);
    printf("  Enter payment amount (Rs.) : ");

    long long amt = read_amount();
    if (amt <= 0)                    { print_error("Invalid amount.");                    return; }
    if (amt > bank[i].balance)       { print_error("Amount exceeds account balance.");    return; }
    if (amt > bank[i].creditUsed)    { print_error("Amount exceeds outstanding bill.");   return; }

    bank[i].balance    -= amt;
    bank[i].creditUsed -= amt;

    char msg[TX_MSG_LEN], notice[TX_MSG_LEN];
    snprintf(msg,    sizeof(msg),    "Credit bill payment: Rs. %.2f", amt / 100.0);
    snprintf(notice, sizeof(notice),
             "Rs. %.2f paid. Remaining bill: Rs. %.2f",
             amt / 100.0, bank[i].creditUsed / 100.0);
    addTransaction(i, msg);
    print_success(notice);
}

/* ═══════════════════════════════════════════════════════════
   CHANGE PIN
   ═══════════════════════════════════════════════════════════ */
void changePIN(int i) {
    print_section("CHANGE PIN");

    printf("\n  Enter current PIN : ");
    int oldPin = maskedPIN();
    if (oldPin == -1) { print_error("Invalid PIN format."); return; }

    unsigned char hash[32];
    hashPIN(oldPin, bank[i].salt, hash);
    if (memcmp(hash, bank[i].pinHash, 32) != 0) {
        print_error("Incorrect PIN. Request cancelled.");
        return;
    }

    printf("  Enter new PIN     : ");
    int newPin = maskedPIN();
    if (newPin == -1) { print_error("Invalid PIN. Must be exactly 4 digits."); return; }

    printf("  Confirm new PIN   : ");
    int confirmPin = maskedPIN();
    if (confirmPin != newPin) {
        print_error("PINs do not match. Request cancelled.");
        return;
    }

    hashPIN(newPin, bank[i].salt, bank[i].pinHash);
    print_success("PIN changed successfully.");
}

/* ═══════════════════════════════════════════════════════════
   CREATE ACCOUNT
   New: minimum opening deposit prompt.
   ═══════════════════════════════════════════════════════════ */
void create(void) {
    if (accCount >= MAX) {
        print_error("Bank capacity full. Cannot open new accounts.");
        return;
    }

    struct Account *a = &bank[accCount];
    memset(a, 0, sizeof(struct Account));

    print_section("OPEN NEW ACCOUNT");
    printf("\n  Full Name : ");
    scanf(" %49[^\n]", a->name);
    int ch; while ((ch = getchar()) != '\n' && ch != EOF);

    /* PIN — loop until valid matching pair entered */
    int pin = -1;
    for (int tries = 0; tries < 3; tries++) {
        printf("  Enter PIN     : ");
        int p = maskedPIN();

        if (p == -1) {
            print_error("PIN must be exactly 4 digits. Try again.");
            continue;
        }

        printf("  Confirm PIN   : ");
        int confirm = maskedPIN();

        if (confirm == -1) {
            print_error("Invalid confirm PIN. Try again.");
            continue;
        }

        if (p != confirm) {
            print_error("PINs do not match. Try again.");
            continue;
        }

        pin = p;   /* success — set the outer variable */
        break;
    }

    if (pin == -1) {
        print_error("Could not set PIN after 3 attempts. Account creation aborted.");
        return;
    }

    if (RAND_bytes(a->salt, 16) != 1) {
        print_error("System error: secure random generation failed.");
        return;
    }

    hashPIN(pin, a->salt, a->pinHash);

    a->accNo       = genNum(12);
    a->debitCard   = genNum(16);
    a->creditCard  = genNum(16);
    a->creditLimit = DEFAULT_CREDIT_LIMIT;

    print_account_created(a->accNo, a->debitCard, a->creditCard, a->creditLimit);

    printf(DIM "  Press Enter to continue (please note your details above)..." RESET);
    getchar();

    accCount++;
    save();
}
/* ═══════════════════════════════════════════════════════════
   HOME / DASHBOARD
   ═══════════════════════════════════════════════════════════ */
void home(int i) {
    int c;
    do {
        processInterest(i);
        print_dashboard(bank[i].name, bank[i].accNo);

        if (!read_int(&c)) {
            print_error("Please enter a number.");
            continue;
        }

        switch (c) {
            case 1:  print_balance(bank[i].balance, bank[i].creditUsed, bank[i].creditLimit); break;
            case 2:  deposit(i);          break;
            case 3:  withdraw(i);         break;
            case 4:  transfer(i);         break;
            case 5:  swipeDebit(i);       break;
            case 6:  swipeCredit(i);      break;
            case 7:  payCredit(i);        break;
            case 8:  showTransactions(i); break;
            case 9:  miniStatement(i);    break;
            case 10: changePIN(i);        break;
            case 11: print_success("Logged out successfully. Goodbye!"); break;
            default: print_error("Invalid option. Choose 1–11.");
        }

        save();

        if (c != 11) pause_for_user();

    } while (c != 11);
}

/* ═══════════════════════════════════════════════════════════
   LOGIN
   ═══════════════════════════════════════════════════════════ */
void login(void) {
    print_section("ACCOUNT LOGIN");

    long long n;
    printf("\n  Account Number : ");
    scanf("%lld", &n);
    int ch; while ((ch = getchar()) != '\n' && ch != EOF);

    int i = find(n);
    if (i == -1) {
        print_error("Account not found. Please check your account number.");
        return;
    }

    if (bank[i].locked) {
        print_error("Account LOCKED — too many failed PIN attempts.");
        printf(SYM_INFO "  Visit your nearest Nova Bank branch to unlock.\n");
        return;
    }

    int attempts = 0;
    while (attempts < MAX_LOGIN_ATTEMPTS) {
        printf("  PIN (%d attempt(s) left) : ", MAX_LOGIN_ATTEMPTS - attempts);
        int p = maskedPIN();

        if (p == -1) {
            print_error("Invalid PIN format. Must be 4 digits.");
            attempts++;
            continue;
        }

        unsigned char hash[32];
        hashPIN(p, bank[i].salt, hash);

        if (memcmp(hash, bank[i].pinHash, 32) == 0) {
            bank[i].failedAttempts = 0;   /* reset on success */
            print_success("Authentication successful. Welcome back!");
            home(i);
            return;
        }

        attempts++;
        char warn[64];
        snprintf(warn, sizeof(warn),
                 "Wrong PIN. %d attempt(s) remaining.", MAX_LOGIN_ATTEMPTS - attempts);
        print_warning(warn);
    }

    bank[i].locked = 1;
    save();
    print_error("Too many failed attempts. Account is now LOCKED.");
    printf(SYM_INFO "  Contact Nova Bank support to unlock your account.\n");
}
