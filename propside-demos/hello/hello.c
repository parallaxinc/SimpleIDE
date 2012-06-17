/*
 * This is a non-traditional hello demo for Propeller-GCC.
 * The demo repeats printing every 100ms with the iteration.
 * It uses waitcnt instead of sleep so it will fit in a COG.
 */
#include <stdio.h>
#include <propeller.h>

int main(void)
{
    int n = 1;
    while(1) {
        waitcnt(CLKFREQ/10+CNT);
        printf("Hello World %d\n", n);
        n++;
    }
    return 0;
}
