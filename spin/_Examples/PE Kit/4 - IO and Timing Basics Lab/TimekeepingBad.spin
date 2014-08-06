''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: TimekeepingBad.spin

CON

    _xinfreq = 5_000_000                     
    _clkmode = xtal1 + pll1x

VAR

    long seconds                

PUB BadTimeCount

    dira[4]~~

    repeat
       waitcnt(clkfreq + cnt)
       seconds ++
       ! outa[4]