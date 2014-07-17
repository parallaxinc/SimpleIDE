''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' ButtonBlink.spin

PUB Main | time

    Repeat

       time := ButtonTime(23)
       Blink(4, time, 10)
       
PUB Blink(pin, rate, reps)

    dira[pin]~~
    outa[pin]~
    
    repeat reps * 2
       waitcnt(rate/2 + cnt)
       !outa[pin]

PUB ButtonTime(pin) : dt | t1, t2

    repeat until ina[pin]
    t1 := cnt
    repeat while ina[pin]
    t2 := cnt
    dt := t2 - t1