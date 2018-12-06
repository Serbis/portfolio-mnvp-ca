#include <printf.h>
#include <stdlib.h>
#include <stdio.h>
#include "../include/asserts.h"


void assertTrue(bool expr, char *assertName) {
    if (!expr) {
        printf("Assert %s failed", assertName);
        exit(0);
    }
}