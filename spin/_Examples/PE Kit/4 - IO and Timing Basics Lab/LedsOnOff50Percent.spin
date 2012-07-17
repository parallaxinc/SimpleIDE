''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: LedsOnOff50Percent.spin
''Leds alternate on/off 50% of
''the time.

PUB BlinkLeds

    dira[4..9]~~

    repeat

        outa[4..9] := %100001
        waitcnt(clkfreq/4 + cnt)
        outa[4..9] := %011110
        waitcnt(clkfreq/4 + cnt)