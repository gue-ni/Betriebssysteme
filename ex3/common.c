/**
 * @file common.c
 * @author Jakob G. Maier <e11809618@student.tuwien.ac.at>
 * @date 10.01.2020
 * 
 * @brief Implements error handling function
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

void error_exit(char *p, char *msg)
{
    fprintf(stderr, "[%s] %s\n", p, msg);
    exit(EXIT_FAILURE);
}
