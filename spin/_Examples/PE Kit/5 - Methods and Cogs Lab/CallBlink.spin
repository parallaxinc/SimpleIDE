''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' CallBlink.spin

PUB Main

    repeat
       outa[9] := dira[9] := 1
       repeat until ina[23]
       outa[9] := 0
       Blink
       waitcnt(clkfreq/2*3 + cnt)

PUB Blink | pin, rate, reps

    pin  := 4
    rate := clkfreq/3
    reps := 9

    dira[pin]~~
    outa[pin]~
    
    repeat reps * 2
       waitcnt(rate/2 + cnt)
       !outa[pin]