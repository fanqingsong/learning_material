/* main.c */
#include "a.h"
#define EXIT_SUCCESS 0

extern void function_two();
extern void function_three();

int main()
{
    function_two();
    function_three();
    exit (EXIT_SUCCESS);
}
