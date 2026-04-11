#ifndef UTILS_H
#define UTILS_H

#include "bank.h"   /* for PIN_MIN / PIN_MAX */

int       read_int(int *out);
int       read_double(double *out);
int       maskedPIN(void);
long long genNum(int digits);
void      hashPIN(int pin, unsigned char *salt, unsigned char *output);

#endif /* UTILS_H */
