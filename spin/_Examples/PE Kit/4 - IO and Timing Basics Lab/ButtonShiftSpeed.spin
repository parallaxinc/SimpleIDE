''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
'' File: ButtonShiftSpeed.spin
'' LED pattern is shifted left to right at variable speeds controlled by pushbuttons.

VAR

    Byte pattern, divide

PUB ShiftLedsLeft

    dira[9..4] ~~
    divide := 5

    repeat
       
       if pattern == 0
          pattern := %11000000

       if ina[22] == 1
          divide ++
          divide <#= 254
       elseif ina[21] == 1
          divide --
          divide #>= 1

       waitcnt(clkfreq/divide + cnt)
       outa[9..4] := pattern
       pattern >>= 1