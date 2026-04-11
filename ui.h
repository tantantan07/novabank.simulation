#ifndef UI_H
#define UI_H

#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── ANSI colours ───────────────────────────────── */
#define RESET    "\033[0m"
#define BOLD     "\033[1m"
#define DIM      "\033[2m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define CYAN     "\033[36m"
#define WHITE    "\033[37m"

/* ── Symbols ────────────────────────────────────── */
#define SYM_OK    GREEN  "  [✔] " RESET
#define SYM_ERR   RED    "  [✘] " RESET
#define SYM_WARN  YELLOW "  [!] " RESET
#define SYM_INFO  CYAN   "  [i] " RESET
#define SYM_ARROW CYAN   "   >> " RESET
#define SYM_MONEY GREEN  "  [₹] " RESET

/* ── Dividers ───────────────────────────────────── */
#define LINE_D "  ════════════════════════════════════════════════════\n"
#define LINE_S "  ────────────────────────────────────────────────────\n"
#define LINE_DOT "  ····················································\n"

/* ── Clear screen ───────────────────────────────── */
#define CLEAR() printf("\033[2J\033[H")

/* ── Pause helper (replaces getchar/getchar hack) ── */
static inline void pause_for_user(void) {
    printf("\n" DIM "  Press Enter to continue..." RESET);
    int c;
    /* drain everything up to and including the next newline */
    while ((c = getchar()) != '\n' && c != EOF);
    getchar();
}

/* ── Feedback printers ──────────────────────────── */
static inline void print_success(const char *msg) {
    printf("\n" SYM_OK  BOLD GREEN  "%s" RESET "\n", msg);
}
static inline void print_error(const char *msg) {
    printf("\n" SYM_ERR BOLD RED    "%s" RESET "\n", msg);
}
static inline void print_warning(const char *msg) {
    printf("\n" SYM_WARN BOLD YELLOW "%s" RESET "\n", msg);
}
static inline void print_info(const char *msg) {
    printf("\n" SYM_INFO CYAN "%s" RESET "\n", msg);
}

/* ── Section header ─────────────────────────────── */
static inline void print_section(const char *title) {
    printf("\n");
    printf(CYAN LINE_D RESET);
    printf(BOLD CYAN "    ◈  %s\n" RESET, title);
    printf(CYAN LINE_S RESET);
}

/* ── Banner ─────────────────────────────────────── */
static inline void print_banner(void) {
    CLEAR();
    printf("\n");
    printf(CYAN "  ╔══════════════════════════════════════════════════╗\n" RESET);
    printf(CYAN "  ║" RESET BOLD  "       🏦  NOVA BANK  —  Digital Banking System     " CYAN "║\n" RESET);
    printf(CYAN "  ║" RESET DIM   "              Secure · Simple · Smart               " CYAN "║\n" RESET);
    printf(CYAN "  ╚══════════════════════════════════════════════════╝\n" RESET);
    printf("\n");
}

/* ── Main menu ──────────────────────────────────── */
static inline void print_main_menu(void) {
    print_banner();
    printf(BOLD "  Welcome to Nova Bank\n" RESET);
    printf(DIM  "  Please select an option to continue\n" RESET);
    printf("\n");
    printf(CYAN LINE_S RESET);
    printf(BOLD "    [ 1 ]" RESET "  Open New Account\n");
    printf(BOLD "    [ 2 ]" RESET "  Login to Account\n");
    printf(BOLD "    [ 3 ]" RESET "  Exit\n");
    printf(CYAN LINE_S RESET);
    printf("\n" SYM_ARROW " ");
}

/* ── Dashboard ──────────────────────────────────── */
static inline void print_dashboard(const char *name, long long accNo) {
    CLEAR();
    printf("\n");
    printf(CYAN "  ╔══════════════════════════════════════════════════╗\n" RESET);
    printf(CYAN "  ║" RESET BOLD GREEN "  ✔  Logged In                                      " CYAN "║\n" RESET);
    printf(CYAN "  ║" RESET "  Welcome back, " BOLD YELLOW "%-33s" RESET CYAN "║\n", name);
    printf(CYAN "  ║" RESET DIM   "  Account No: %-36lld" RESET CYAN "║\n", accNo);
    printf(CYAN "  ╚══════════════════════════════════════════════════╝\n" RESET);
    printf("\n");
    printf(CYAN LINE_S RESET);
    printf(BOLD "    [ 1  ]" RESET "  Check Balance\n");
    printf(BOLD "    [ 2  ]" RESET "  Deposit Money\n");
    printf(BOLD "    [ 3  ]" RESET "  Withdraw Money\n");
    printf(BOLD "    [ 4  ]" RESET "  Transfer Money\n");
    printf(BOLD "    [ 5  ]" RESET "  Swipe Debit Card\n");
    printf(BOLD "    [ 6  ]" RESET "  Swipe Credit Card\n");
    printf(BOLD "    [ 7  ]" RESET "  Pay Credit Bill\n");
    printf(BOLD "    [ 8  ]" RESET "  Transaction History\n");
    printf(BOLD "    [ 9  ]" RESET "  Mini Statement\n");
    printf(BOLD "    [ 10 ]" RESET "  Change PIN\n");
    printf(BOLD RED "    [ 11 ]" RESET "  Logout\n");
    printf(CYAN LINE_S RESET);
    printf("\n" SYM_ARROW " ");
}

/* ── Balance card ───────────────────────────────── */
static inline void print_balance(long long balance, long long used, long long limit) {
    print_section("ACCOUNT OVERVIEW");
    printf("\n");
    printf(SYM_MONEY BOLD "  Available Balance  : " RESET GREEN BOLD "  Rs. %12.2f\n" RESET, balance / 100.0);
    printf(DIM LINE_DOT RESET);
    printf(SYM_INFO  "  Credit Used        : " YELLOW  "  Rs. %12.2f\n" RESET, used   / 100.0);
    printf(SYM_INFO  "  Credit Limit       : " CYAN    "  Rs. %12.2f\n" RESET, limit  / 100.0);
    printf(SYM_INFO  "  Credit Available   : " GREEN   "  Rs. %12.2f\n" RESET, (limit - used) / 100.0);
    printf("\n");
    printf(CYAN LINE_S RESET);
}

/* ── Account-created card ───────────────────────── */
static inline void print_account_created(long long accNo, long long debit,
                                          long long credit, long long limitPaise) {
    print_section("ACCOUNT CREATED SUCCESSFULLY");
    printf("\n");
    printf(SYM_OK BOLD "  Your account is ready!\n\n" RESET);
    printf(CYAN "  ┌──────────────────────────────────────────────────┐\n" RESET);
    printf(CYAN "  │" RESET "  Account Number   : " BOLD YELLOW "%lld"  RESET CYAN "\n", accNo);
    printf(CYAN "  │" RESET "  Debit  Card No   : " BOLD        "%lld"  RESET CYAN "\n", debit);
    printf(CYAN "  │" RESET "  Credit Card No   : " BOLD        "%lld"  RESET CYAN "\n", credit);
    printf(CYAN "  │" RESET DIM "  Credit Limit     : Rs. %.2f" RESET CYAN "\n", limitPaise / 100.0);
    printf(CYAN "  └──────────────────────────────────────────────────┘\n" RESET);
    printf(SYM_WARN YELLOW "  Save your account details somewhere safe!\n" RESET);
    printf("\n");
}

/* ── Transaction row (colour-coded) ────────────────
   + = green, - = red, neutral = dim                */
static inline void print_tx_row(int idx, const char *tx) {
    if (strstr(tx, "Deposited") || strstr(tx, "Received") || strstr(tx, "Interest"))
        printf(GREEN "  %3d │ " RESET "%-54s\n", idx, tx);
    else if (strstr(tx, "Withdrew") || strstr(tx, "Sent") ||
             strstr(tx, "payment")  || strstr(tx, "bill"))
        printf(RED   "  %3d │ " RESET "%-54s\n", idx, tx);
    else
        printf(DIM   "  %3d │ " RESET "%-54s\n", idx, tx);
}

#endif /* UI_H */
