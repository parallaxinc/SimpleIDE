''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: IncrementUntilCondition.spin

PUB BlinkLeds

    dira[4..9]~~

    repeat until outa[9..4]++ == 19
        
        waitcnt(clkfreq/2 + cnt)

    repeat