''This code example is from Propeller Education Kit Labs: Fundamentals, v1.2.
''A .pdf copy of the book is available from www.parallax.com, and also through
''the Propeller Tool software's Help menu (v1.2.6 or newer).
''
''File: ActuatedStreetlightsEW.spin
''A high speed prototype of a N/S E/W streetlight controller.

PUB StreetLightsActuatedEW

    dira[9..4]~~                             ' Set LED I/O pins to output
    
    repeat                                   ' Main loop

       outa[4..9] := %001100                 ' N/S green, E/W red
       repeat until ina[21]                  ' Car on E/W street 
       waitcnt(clkfreq * 3 + cnt)            ' 8 s
       outa[4..9] := %010100                 ' N/S yellow, E/W red
       waitcnt(clkfreq * 3  + cnt)           ' 3 s
       outa[4..9] := %100001                 ' N/S red, E/W green
       waitcnt(clkfreq * 8 + cnt)            ' 8 s
       outa[4..9] := %100010                 ' N/S red, E/W yellow
       waitcnt(clkfreq * 3 + cnt)            ' 3 s