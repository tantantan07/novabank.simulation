#include <stdio.h>
#include "include/bank.h"

int main() {
    FILE *fp = fopen("bank.dat", "rb");
    if (!fp) {
        printf("No data file found.\n");
        return 0;
    }

    int count;
    fread(&count, sizeof(int), 1, fp);

    struct Account a;

    printf("\n--- DEBUG VIEW ---\n");

    for (int i = 0; i < count; i++) {
        fread(&a, sizeof(struct Account), 1, fp);

        printf("\nAccount %d\n", i + 1);
        printf("Name: %s\n", a.name);
        printf("AccNo: %lld\n", a.accNo);
        printf("Balance: %.2f\n", a.balance);

        printf("Salt: ");
        for (int k = 0; k < 16; k++)
            printf("%02x", a.salt[k]);

        printf("\nPIN Hash: ");
        for (int k = 0; k < 32; k++)
            printf("%02x", a.pinHash[k]);

        printf("\n");
    }

    fclose(fp);
    return 0;
}
