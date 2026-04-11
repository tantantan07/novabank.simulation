#include <stdio.h>
#include <string.h>
#include "bank.h"
#include "file.h"
#include "ui.h"

#define DATA_FILE "bank.dat"
#define TEMP_FILE "bank.tmp"

/* ── Save ────────────────────────────────────────────────────────────────────
 * Write to a temp file first, then atomically rename.
 * This protects against data corruption if the program crashes mid-write.
 * -------------------------------------------------------------------------- */
void save(void) {
    FILE *fp = fopen(TEMP_FILE, "wb");
    if (!fp) {
        print_error("Critical: Could not open temp file for saving. Data NOT saved.");
        return;
    }

    if (fwrite(&g_bank.count, sizeof(int), 1, fp) != 1 ||
        fwrite(g_bank.accounts, sizeof(struct Account), g_bank.count, fp)
            != (size_t)g_bank.count) {
        print_error("Critical: Write error. Data may be incomplete.");
        fclose(fp);
        return;
    }

    fclose(fp);

    /* Atomic replace: rename temp → real file */
    if (rename(TEMP_FILE, DATA_FILE) != 0) {
        print_error("Critical: Could not finalise save. Data may be lost.");
    }
}

/* ── Load ─────────────────────────────────────────────────────────────────── */
void load(void) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return;   /* first run – no file yet, that's fine */

    if (fread(&g_bank.count, sizeof(int), 1, fp) != 1) {
        g_bank.count = 0;
        fclose(fp);
        return;
    }

    /* Guard against corrupt count value */
    if (g_bank.count < 0 || g_bank.count > MAX) {
        g_bank.count = 0;
        fclose(fp);
        print_warning("Data file appears corrupt. Starting fresh.");
        return;
    }

    fread(g_bank.accounts, sizeof(struct Account), g_bank.count, fp);
    fclose(fp);
}
