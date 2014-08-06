''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: TimeCounter.spin

CON

    _xinfreq = 5_000_000                     
    _clkmode = xtal1 + pll1x

VAR

    long seconds, minutes, hours, days, dT, T

PUB GoodTimeCount

    dira[9..4]~~

    dT := clkfreq
    T  := cnt

    repeat
    
       T += dT
       waitcnt(T)
       seconds++

       if seconds // 60 == 0
          minutes++
          if minutes == 60
             minutes := 0 
       if seconds // 3600 == 0
          hours++
          if hours == 24
             hours := 0
       if seconds // 86400 == 0
          days++
       
       outa[9..4] := seconds