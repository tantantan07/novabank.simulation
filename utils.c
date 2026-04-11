#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "utils.h"
#include "ui.h"
#include <termios.h>
#include <unistd.h>

/* ── Safe integer input ─────────────────────────────────────────────────── */
int read_int(int *out) {
    char buf[32];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;

    char *end;
    long val = strtol(buf, &end, 10);

    if (end == buf || (*end != '\n' && *end != '\0')) return 0;

    *out = (int)val;
    return 1;
}

/* ── Safe float input ───────────────────────────────────────────────────── */
int read_double(double *out) {
    char buf[32];
    if (!fgets(buf, sizeof(buf), stdin)) return 0;

    char *end;
    double val = strtod(buf, &end);

    if (end == buf || (*end != '\n' && *end != '\0')) return 0;

    *out = val;
    return 1;
}

/* ── PIN reader with validation ─────────────────────────────────────────── */
int maskedPIN(void) {
    struct termios old, new;

    /* PRINT PROMPT FIRST */
    printf("Enter 4-digit PIN: ");
    fflush(stdout);

    tcgetattr(STDIN_FILENO, &old);
    new = old;

    new.c_lflag &= ~(ECHO | ICANON);
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);

    char digits[4];
    int count = 0;

    while (count < 4) {
        char c;
        read(STDIN_FILENO, &c, 1);

        /* Backspace */
        if ((c == 127 || c == '\b') && count > 0) {
            count--;
            printf("\b \b");
            fflush(stdout);
            continue;
        }

        /* Only digits */
        if (c < '0' || c > '9') continue;

        digits[count++] = c;
        printf("*");
        fflush(stdout);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);
    printf("\n");

    int pin = 0;
    for (int k = 0; k < 4; k++)
        pin = pin * 10 + (digits[k] - '0');

    return (pin >= 1000 && pin <= 9999) ? pin : -1;
}

/* ── Cryptographically secure N-digit number ────────────────────────────── */
long long genNum(int d) {
    unsigned char buf[8];
    RAND_bytes(buf, sizeof(buf));

    unsigned long long r = 0;
    for (int i = 0; i < 8; i++)
        r = r * 256 + buf[i];

    long long mod = 1;
    for (int i = 0; i < d; i++) mod *= 10;

    long long n = (long long)(r % (unsigned long long)mod);
    long long minVal = mod / 10;

    if (n < minVal) n += minVal;

    return n;
}

/* ── SHA-256 PIN hash with salt ─────────────────────────────────────────── */
void hashPIN(int pin, unsigned char *salt, unsigned char *output) {
    unsigned char input[20];  // 16 salt + 4 pin

    memcpy(input, salt, 16);
    memcpy(input + 16, &pin, sizeof(int));

    SHA256(input, 16 + sizeof(int), output);
}