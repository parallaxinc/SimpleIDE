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
        //waitcnt(CLKFREQ/5+CNT);
        //waitcnt(CLKFREQ/50+CNT);   // faster
        //waitcnt(CLKFREQ/500+CNT);  // murderous
        waitcnt(CLKFREQ/5000+CNT);   // redrum
        printf("Hello World %d\n", n);
        n++;
    }
    return 0;
}