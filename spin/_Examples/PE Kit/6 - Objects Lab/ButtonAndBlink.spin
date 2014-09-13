''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: ButtonAndBlink.spin
'' Example object with two methods

PUB ButtonTime(pin): delta | time1, time2

    repeat until ina[pin] == 1
    time1 := cnt
    repeat until ina[pin] == 0
    time2 := cnt
    delta := time2 - time1


PUB Blink( pin, rate, reps)

    dira[pin]~~
    outa[pin]~
    
    repeat reps * 2
       waitcnt(rate/2 + cnt)
       !outa[pin]