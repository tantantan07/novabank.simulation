#include <stdio.h>
#include "bank.h"
#include "account.h"
#include "file.h"
#include "utils.h"
#include "ui.h"

/* Single global bank instance — defined here, declared extern in bank.h */
struct Bank g_bank = {0};

int main(void) {
    load();

    int c;
    do {
        print_main_menu();

        if (!read_int(&c)) {
            print_error("Please enter a valid number.");
            continue;
        }

        switch (c) {
            case 1: create(); break;
            case 2: login();  break;
            case 3:
                print_banner();
                printf(SYM_INFO "  Thank you for banking with " BOLD CYAN "Nova Bank" RESET ".\n");
                printf(DIM      "  Have a great day!\n\n" RESET);
                break;
            default:
                print_error("Invalid choice. Please enter 1, 2, or 3.");
        }
    } while (c != 3);

    save();
    return 0;
}
