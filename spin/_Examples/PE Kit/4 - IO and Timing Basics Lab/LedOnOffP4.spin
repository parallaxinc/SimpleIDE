''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: LedOnOffP4.spin

PUB LedOnOff

    dira[4] := 1
    
    repeat
        outa[4] := 1
        waitcnt(clkfreq/4 + cnt)
        outa[4] := 0
        waitcnt(clkfreq/4*3 + cnt)