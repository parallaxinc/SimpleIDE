''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: ShiftRightP9toP4.spin
'' Demonstrates the right shift operator and if statement.

PUB ShiftLedsLeft

    dira[9..4] ~~

    repeat
       
       if outa[9..4] == 0
          outa[9..4] := %100000

       waitcnt(clkfreq/10 + cnt)
       outa[9..4] >>= 1