''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: LedsOnOff50PercentAgain.spin
''Leds alternate on/off 50% of
''the time with the ! operator.

PUB BlinkLeds

    dira[4..9]~~
    outa[4..9] := %100001

    repeat

        !outa[4..9] 
        waitcnt(clkfreq/4 + cnt)