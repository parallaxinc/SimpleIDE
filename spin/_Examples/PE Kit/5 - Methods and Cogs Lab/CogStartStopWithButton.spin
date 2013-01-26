''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' Launches methods into cogs and stops the cogs within loop structures that
'' are advanced by pushbuttons.

VAR

    long stack[60]

PUB ButtonBlinkTime | time, index, cog[6]

    repeat
    
       repeat index from 0 to 5
          time := ButtonTime(23)
          cog[index] := cognew(Blink(index + 4, time, 1_000_000), @stack[index * 10])
       
       repeat index from 5 to 0
          ButtonTime(23)
          cogstop(cog[index]) 
             
       
PUB Blink( pin, rate, reps)

    dira[pin]~~
    outa[pin]~
    
    repeat reps * 2
       waitcnt(rate/2 + cnt)
       !outa[pin]
       

PUB ButtonTime(pin) : delta | time1, time2

    repeat until ina[pin] == 1
    time1 := cnt
    repeat until ina[pin] == 0
    time2 := cnt
    delta := time2 - time1