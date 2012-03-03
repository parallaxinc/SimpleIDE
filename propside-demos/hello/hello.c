/*
 * This demo uses waitcnt instead of sleep so it will fit in a COG.
 * It also repeats printing every 200ms and prints the iteration.
 * Some computers may not be fast enough for the terminal to catch
 * the serial input after loading the Propeller, so you may not see
 * "Hello World 1", but you will see something on the terminal.
 */
#include <stdio.h>
#include <propeller.h>

int main(void)
{
    int n = 1;
    while(1) {
        waitcnt(CLKFREQ/5+CNT);
        printf("Hello World %d\n", n);
        n++;
    }
    return 0;
}
