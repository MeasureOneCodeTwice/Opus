#include "testlib.h"
#include <stdio.h>


#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[39m"


bool verify_fcn(int expression, int* tests_passed, int* tests_failed, char* expression_text) {
    int testnum = *tests_passed + *tests_failed + 1;
    printf("Test %d ", testnum);
    if (expression) {
        (*tests_passed)++;
         printf("%spassed%s\n", GREEN, RESET);
    } else {
        (*tests_failed)++;
        printf("%sfailed%s!\n   '%s' evaluated to false.\n",
                RED, RESET, expression_text);
    }
    return expression;
}

void summary(int* tests_passed, int* tests_failed) {
    int num_tests = *tests_passed + *tests_failed;
    printf("\n\n");
    if (num_tests  == 0 ) 
        printf("No tests ran...");
    else if (*tests_failed == 0)
        printf("%sAll tests passed!%s\n", GREEN, RESET);
    else if(*tests_passed == 0)
        printf("%sAll tests failed%s, you fucking muppet.\n", RED, RESET);
    else
        printf("%d of %d %stests failed%s.\n", *tests_failed, num_tests, RED, RESET);
}
