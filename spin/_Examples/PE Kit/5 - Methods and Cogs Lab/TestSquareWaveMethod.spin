''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: TestSquareWaveMethod.spin

CON

    _xinfreq = 5_000_000                     
    _clkmode = xtal1 + pll16x


VAR

    long swStack[120] 
    byte swCog[3]
    

PUB TestSquareWave

    swCog[0] := cognew(SquareWave(5, clkfreq/20, clkfreq/10), @swStack)
    swCog[1] := cognew(SquareWave(6, clkfreq/100, clkfreq/5), @swStack[40])
    swCog[2] := cognew(SquareWave(9, clkfreq/2000, clkfreq/500), @swStack[80])

PUB SquareWave(pin, tHigh, tCycle) : success | tH, tC

    outa[pin]~
    dira[pin]~~

    tC := cnt

    repeat
       outa[pin]~~
       tH := tC + tHigh
       tC += tCycle
       waitcnt(tH)
       outa[pin]~
       waitcnt(tC)