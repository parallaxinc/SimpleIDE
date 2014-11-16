#include <unistd.h>
#include <propeller.h>
__attribute__((constructor)) void init(void) {
	/* assert tx pin low for QuickStart */
	OUTA &= ~(1<<30);
	DIRA |=  (1<<30); 
	/* wait a second for terminal to start */
	//sleep(1);
	waitcnt(CLKFREQ+CNT);
}