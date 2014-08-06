''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: PushbuttonLedTest.spin
''Test program for the Propeller Education Lab "PE Platform Setup"

CON

  _clkmode        = xtal1 + pll16x           ' Feedback and PLL multiplier
  _xinfreq        = 5_000_000                ' External oscillator = 5 MHz

  LEDs_START      = 0                        ' Start of I/O pin group for on/off signals
  LEDs_END        = 15                       ' End of I/O pin group for on/off signals
  PUSHBUTTON      = 18                       ' Pushbutton Input Pin

PUB ButtonBlinkSpeed                         ' Main method

  '' Sends on/off (3.3 V / 0 V) signals at approximately 2 Hz. 

  dira[LEDs_START..LEDs_END]~~               ' Set entire pin group to output

  repeat                                     ' Endless loop

    ! outa[LEDs_START..LEDs_END]             ' Change the state of pin group
    
    if ina[PUSHBUTTON] == 1                  ' If pushbutton pressed
      waitcnt(clkfreq / 4 + cnt)             ' Wait 1/4 second -> 2 Hz
    else                                     ' If pushbutton not pressed
      waitcnt(clkfreq / 20 + cnt)            ' Wait 1/20 second -> 10 Hz
      