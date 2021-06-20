#include "utils.h"


// Convert int to str
char* itoa(int n) {
    int length = snprintf(NULL, 0, "%d", n);
    char* string = calloc(length + 1, sizeof(char));
    sprintf(string, "%d", n);
    return string;  // IMPORTANT: Remember to free memory from this pointer
}

// Concatenate array of str
char* concatenate(char** str_array, int size) {

    // Calculate final string size
    int total_length = 0;
    for (int s = 0; s < size; s++) {
        total_length += strlen(str_array[s]);
    }

    // Generate new string
    char* new_string = calloc(total_length + 1, sizeof(char));
    for (int s = 0; s < size; s++) {
        strcat(new_string, str_array[s]);
    }

    return new_string;  // IMPORTANT: Remember to free memory from this pointer
}

// Get ceil
int div_ceil(int a, int b) {
    int result = a / b;
    if (result * b < a) {
        result++;
    }
    return result;
}

// Get min
int int_min(int a, int b) {
    int min;
    if (a <= b) {
        min = a;
    } else {
        min = b;
    }
    return min;
}
