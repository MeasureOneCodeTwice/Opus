#include <stdbool.h>
bool verify_fcn(
        int expression, int* tests_passed,
        int* tests_failed, char* expression_str
        );
#define verify(e, desc, ...)\
    if(!verify_fcn(e, &tests_passed, &tests_failed, #e)) {\
        fprintf(stderr, "    " desc "\n", ##__VA_ARGS__);\
    }

void summary(int* tests_passed, int* tests_failed);
