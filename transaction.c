#include <stdio.h>
#include <string.h>
#include "bank.h"
#include "transaction.h"
#include "ui.h"
#include <time.h> 

/* ── Add a transaction entry ──────────────────────────────────────────────── */
void addTransaction(int i, const char *msg) {
    if (bank[i].txCount < MAX_TX) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "%d %b %Y | %H:%M", t);

        char finalMsg[100];
        snprintf(finalMsg, sizeof(finalMsg), "[%s] %s", timestamp, msg);

        strncpy(bank[i].transactions[bank[i].txCount], finalMsg, 99);
        bank[i].transactions[bank[i].txCount][99] = '\0';

        bank[i].txCount++;
    } else {
        printf("Warning: Transaction log full.\n");
    }
}

/* ── Table header / footer helpers ─────────────────────────────────────────── */
static void print_tx_table_header(void) {
    printf("\n");
    printf(CYAN "  ┌─────┬────────────────────────────────────────────────────┐\n" RESET);
    printf(CYAN "  │" RESET BOLD " No. │ Description                                        " CYAN "│\n" RESET);
    printf(CYAN "  ├─────┼────────────────────────────────────────────────────┤\n" RESET);
}

static void print_tx_table_footer(int count) {
    printf(CYAN "  └─────┴────────────────────────────────────────────────────┘\n" RESET);
    printf(DIM  "  Total shown: %d\n" RESET, count);
}

/* ── Full history ────────────────────────────────────────────────────────── */
void showTransactions(int i) {
    print_section("FULL TRANSACTION HISTORY");

    if (bank[i].txCount == 0) {
        print_info("No transactions recorded yet.");
        return;
    }

    print_tx_table_header();
    for (int j = 0; j < bank[i].txCount; j++)
        print_tx_row(j + 1, bank[i].transactions[j]);
    print_tx_table_footer(bank[i].txCount);
}

/* ── Mini statement (last 5) ─────────────────────────────────────────────── */
void miniStatement(int i) {
    print_section("MINI STATEMENT  (Last 5 Transactions)");

    if (bank[i].txCount == 0) {
        print_info("No transactions recorded yet.");
        return;
    }

    int start = bank[i].txCount - 5;
    if (start < 0) start = 0;
    int shown = bank[i].txCount - start;

    print_tx_table_header();
    for (int j = start; j < bank[i].txCount; j++)
        print_tx_row(j + 1, bank[i].transactions[j]);
    print_tx_table_footer(shown);
}
