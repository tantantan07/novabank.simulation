#ifndef BANK_H
#define BANK_H

#include <time.h>

/* ── Limits ─────────────────────────────────────── */
#define MAX                  100
#define MAX_TX               100
#define NAME_LEN              50
#define TX_MSG_LEN           120

/* ── Business constants (no more magic numbers) ─── */
#define DEFAULT_CREDIT_LIMIT  5000000   /* paise = Rs. 50,000  */
#define LARGE_TX_THRESHOLD    2000000   /* paise = Rs. 20,000  */
#define INTEREST_TIER_1        1000000   /* paise = Rs. 10,000  */
#define INTEREST_TIER_2        5000000   /* paise = Rs. 50,000  */
#define INTEREST_RATE_1        0.01f
#define INTEREST_RATE_2        0.02f
#define INTEREST_RATE_3        0.03f
#define INTEREST_PERIOD_SECS  (30 * 24 * 60 * 60)
#define PIN_MIN               1000
#define PIN_MAX               9999
#define MAX_LOGIN_ATTEMPTS       3

/*
 * All monetary values stored as INTEGER PAISE (1 Rs = 100 paise)
 * This eliminates float rounding errors on money calculations.
 * Display helper: paise_to_rs(x) converts back for printing.
 */
struct Account {
    long long  accNo;
    char       name[NAME_LEN];
    unsigned char pinHash[32];
    unsigned char salt[16];

    long long  balance;       /* in paise */
    long long  debitCard;
    long long  creditCard;
    long long  creditLimit;   /* in paise */
    long long  creditUsed;    /* in paise */

    time_t     lastInterest;
    int        locked;
    int        failedAttempts; /* track across sessions */

    char       transactions[MAX_TX][TX_MSG_LEN];
    int        txCount;
};

/* Wrap global state in one place */
struct Bank {
    struct Account accounts[MAX];
    int            count;
};

extern struct Bank g_bank;

/* Convenience macro so existing code using bank[i] still works */
#define bank  g_bank.accounts
#define accCount g_bank.count

/* Money helpers */
static inline double paise_to_rs(long long p) { return p / 100.0; }
static inline long long rs_to_paise(double rs) { return (long long)(rs * 100 + 0.5); }

#endif /* BANK_H */
