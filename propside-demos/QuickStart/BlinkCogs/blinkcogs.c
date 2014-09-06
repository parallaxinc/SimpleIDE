/*
 * blinkcogs.c
 *
 * Make all propeller cogs blink assigned pins at exactly the
 * same rate and time to demonstrate the precision of the
 * _start_cog_thread method. This program and method uses
 * 8 LMM C program COG "threads" of execution simultaneously.
 *
 * This program should be compiled with the LMM memory model.
 * A printf can be enabled for showing COGs started by changing DPRINTF.
 */

/* To enable printf change this to #define DPRINTF 1 */
#define DPRINTF 0

#include <stdio.h>
#include <propeller.h>

#if DPRINTF
#define printf __simple_printf
#else
#define printf
#endif

#define COGS 8
#define STACK_SIZE 16

static int cog_stack[STACK_SIZE][8];
static _thread_state_t thread_data;

volatile unsigned int wait_time;
volatile unsigned int startcnt;
volatile unsigned int pins;
volatile int syncstart;

/*
 * Set a pin high without affecting other pins.
 * param WCpin = pin number to set high.
 */
void high(int WCpin)
{
    unsigned int bits = 1 << WCpin;
    DIRA |= bits;
    OUTA |= bits;
}

/*
 * Set a pin low without affecting other pins.
 * param WCpin = pin number to set low.
 */
void low(int WCpin)
{
    unsigned int mask = 1 << WCpin;
    DIRA |= mask;
    OUTA &= ~mask;
}

/*
 * toggle thread function gets started in an LMM COG.
 * param arg = pin number to toggle
 */
void do_toggle(void *arg)
{
    int pin = (int) arg;
    unsigned int nextcnt;

    while(syncstart == 0) ; // wait for start signal from main cog
    
    nextcnt = wait_time + startcnt;
    while(1)
    {
        high(pin);
        nextcnt = waitcnt2(nextcnt, wait_time);
        low(pin);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}

int main (int argc, char* argv[])
{
    int n;
    int cog;
    int pin[] = { 16, 17, 18, 19, 20, 21, 22, 23 };
    unsigned int nextcnt;

    wait_time = CLKFREQ/20;

    syncstart = 0;

    for(n = 1; n < COGS; n++) {
        cog = _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle, (void*)pin[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
    }

    startcnt = CNT;
    syncstart = 1;
    nextcnt = wait_time + startcnt;
    while(1)
    {
        high(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
        low(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
    return 0;
}

