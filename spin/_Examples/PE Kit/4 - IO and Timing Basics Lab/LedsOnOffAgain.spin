''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: LedsOnOffAgain.spin
''All LEDS on for 1/4 s and off
''for 3/4 s with post set/clear.

PUB BlinkLeds

    dira[4..9]~~

    repeat

        outa[4..9]~~
        waitcnt(clkfreq/4 + cnt)
        outa[4..9]~
        waitcnt(clkfreq/4*3 + cnt)