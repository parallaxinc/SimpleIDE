''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' BlinkWithCogs.spin

VAR
    long stack[30]

PUB LaunchBlinkCogs 

    cognew(Blink(4, clkfreq/3, 9), @stack[0])
    cognew(Blink(5, clkfreq/7, 21), @stack[10])
    cognew(Blink(6, clkfreq/11, 39), @stack[20])


PUB Blink( pin, rate, reps)

    dira[pin]~~
    outa[pin]~
    
    repeat reps * 2
       waitcnt(rate/2 + cnt)
       !outa[pin]