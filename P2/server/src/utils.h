#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#pragma once


// Convert int to str
char* itoa(int n);

// Concatenate array of str
char* concatenate(char** str_array, int size);

// Get ceil of the division of two numbers
int div_ceil(int a, int b);